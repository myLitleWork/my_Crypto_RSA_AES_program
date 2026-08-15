#Requires -Version 5.1
[CmdletBinding()]
param(
    [string]$BuildDirectory,
    [string]$OutputDirectory,
    [switch]$AllowDirty,
    [string]$CloudRoot,
    [string]$CloudReleaseId,
    [string]$ZipName,
    [switch]$SkipCloudPublish,
    [string]$ReleaseGitCommit,
    [Nullable[bool]]$ReleaseGitDirty,
    [string[]]$ReleaseGitStatus
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$productName = 'ShifRator3'
$markerName = '.shifrator3-release-package'

function Get-Sha256([string]$Path) {
    $sha = [Security.Cryptography.SHA256]::Create()
    $stream = [IO.File]::OpenRead($Path)
    try {
        return ([BitConverter]::ToString($sha.ComputeHash($stream))).
            Replace('-', '').ToLowerInvariant()
    } finally {
        $stream.Dispose()
        $sha.Dispose()
    }
}

function Get-TextSha256([string]$Text) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        $bytes = [Text.Encoding]::UTF8.GetBytes($Text)
        return ([BitConverter]::ToString($sha.ComputeHash($bytes))).
            Replace('-', '').ToLowerInvariant()
    } finally {
        $sha.Dispose()
    }
}

function Assert-PeWin32([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $reader = [IO.BinaryReader]::new($stream)
    try {
        if ($reader.ReadUInt16() -ne 0x5A4D) { throw "Not an MZ file: $Path" }
        $stream.Position = 0x3c
        $peOffset = $reader.ReadInt32()
        if ($peOffset -lt 0 -or $peOffset -gt ($stream.Length - 6)) {
            throw "Invalid PE header offset: $Path"
        }
        $stream.Position = $peOffset
        if ($reader.ReadUInt32() -ne 0x00004550) { throw "Missing PE signature: $Path" }
        if ($reader.ReadUInt16() -ne 0x014c) { throw "Not a Win32 x86 PE: $Path" }
    } finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

function Test-SensitiveRelativePath([string]$RelativePath) {
    $normalized = $RelativePath.Replace('/', '\')
    $parts = @($normalized.Split('\') | Where-Object { $_ -ne '' })
    $sendDirectory = -join ([char[]]@(
        0x041E,0x0442,0x043F,0x0440,0x0430,0x0432,0x043A,0x0430
    ))
    $receiveDirectory = -join ([char[]]@(
        0x041F,0x0440,0x0438,0x0435,0x043C
    ))
    foreach ($part in $parts) {
        if ($part -in @($sendDirectory, $receiveDirectory) -or
            $part -match '^(?i:rsa_keys|keys?|user.?data|logs?|temp|tmp)$') {
            return $true
        }
    }
    $leaf = if ($parts.Count -gt 0) { $parts[$parts.Count - 1] } else { $normalized }
    $extension = [IO.Path]::GetExtension($leaf).ToLowerInvariant()
    if ($extension -in @('.pem','.key','.pfx','.p12','.cer','.crt','.env',
            '.cry','.cry2','.db','.db3','.sqlite','.sqlite3','.log')) { return $true }
    return $leaf -match '(?i)(prikey|private.?key|system.?token|password|credential|secret|fingerprint|request)'
}

function Get-SafeTreeFiles([string]$Root) {
    if (-not (Test-Path -LiteralPath $Root -PathType Container)) { return @() }
    $result = [Collections.Generic.List[IO.FileInfo]]::new()
    $pending = [Collections.Generic.Stack[IO.DirectoryInfo]]::new()
    $pending.Push((Get-Item -LiteralPath $Root))
    while ($pending.Count -gt 0) {
        $directory = $pending.Pop()
        if (($directory.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "Reparse point is forbidden in package input: $($directory.FullName)"
        }
        foreach ($file in Get-ChildItem -LiteralPath $directory.FullName -File -Force) {
            if (($file.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                throw "Reparse point is forbidden in package input: $($file.FullName)"
            }
            $relative = $file.FullName.Substring($Root.Length).TrimStart('\','/')
            if (Test-SensitiveRelativePath $relative) {
                throw "Sensitive file is forbidden in package input: $($file.FullName)"
            }
            $result.Add($file)
        }
        foreach ($child in Get-ChildItem -LiteralPath $directory.FullName -Directory -Force) {
            $relative = $child.FullName.Substring($Root.Length).TrimStart('\','/')
            if (Test-SensitiveRelativePath $relative) {
                throw "Sensitive directory is forbidden in package input: $($child.FullName)"
            }
            $pending.Push($child)
        }
    }
    return @($result)
}

function Copy-SafeTree([string]$SourceRoot, [string]$DestinationRoot) {
    foreach ($file in Get-SafeTreeFiles $SourceRoot) {
        $relative = $file.FullName.Substring($SourceRoot.Length).TrimStart('\','/')
        $destination = Join-Path $DestinationRoot $relative
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
        Copy-Item -LiteralPath $file.FullName -Destination $destination
    }
}

$getNames = & (Join-Path $PSScriptRoot 'Find-WorkspaceReleaseRoot.ps1') `
    -Name 'Get-VclAppVersionNames.ps1' -StartPath $projectRoot
$versionInfo = & $getNames -ProjectRoot $projectRoot -ProductName $productName
$versionDocument = Get-Content -LiteralPath (Join-Path $projectRoot 'app-version.json') `
    -Raw | ConvertFrom-Json
$releaseFingerprintFormat = [int]$versionDocument.releaseFingerprintFormat
$releaseInputSha256 = [string]$versionDocument.releaseInputSha256
if ($releaseFingerprintFormat -lt 1 -or
    $releaseInputSha256 -notmatch '^[0-9a-fA-F]{64}$') {
    throw 'app-version.json has no valid release fingerprint.'
}
$releaseInputSha256 = $releaseInputSha256.ToLowerInvariant()
if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
    $BuildDirectory = Join-Path $projectRoot 'Win32\Release'
}
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $projectRoot (
        Join-Path 'out\package' (Join-Path $versionInfo.ReleaseId $versionInfo.OutputLeaf))
}
if ([string]::IsNullOrWhiteSpace($ZipName)) { $ZipName = $versionInfo.ZipFileName }
if ([string]::IsNullOrWhiteSpace($CloudReleaseId)) { $CloudReleaseId = $versionInfo.ReleaseId }
$BuildDirectory = [IO.Path]::GetFullPath($BuildDirectory)
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$outRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot 'out\package'))
if (-not $OutputDirectory.StartsWith($outRoot + [IO.Path]::DirectorySeparatorChar,
        [StringComparison]::OrdinalIgnoreCase)) {
    throw "Output directory is outside canonical package root: $OutputDirectory"
}

$exe = Join-Path $BuildDirectory 'myProgram.exe'
$runtimeDll = Join-Path $projectRoot 'runtime\myCrypto.dll'
$templateRoot = Join-Path $projectRoot 'deployment\ShifRator3Portable'
$tutorialRoot = Join-Path $projectRoot 'tutorials'
foreach ($required in @($exe, $runtimeDll,
    (Join-Path $templateRoot 'Verify-Package.cmd'),
    (Join-Path $templateRoot 'Verify-Package.ps1'))) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required package input is missing: $required"
    }
}
$null = Get-SafeTreeFiles $templateRoot
$null = Get-SafeTreeFiles $tutorialRoot
Assert-PeWin32 $exe
Assert-PeWin32 $runtimeDll
$peVersion = [Diagnostics.FileVersionInfo]::GetVersionInfo($exe)
if ($peVersion.FileVersion -ne $versionInfo.FileVersion -or
    $peVersion.ProductVersion -ne $versionInfo.Version) {
    throw ('Release executable PE version does not match app-version.json: ' +
        "$($peVersion.FileVersion) / $($peVersion.ProductVersion)")
}

$hasReleaseGitContext = $PSBoundParameters.ContainsKey('ReleaseGitCommit') -or
    $PSBoundParameters.ContainsKey('ReleaseGitDirty') -or
    $PSBoundParameters.ContainsKey('ReleaseGitStatus')
if ($hasReleaseGitContext -and
    (-not $PSBoundParameters.ContainsKey('ReleaseGitCommit') -or
     -not $PSBoundParameters.ContainsKey('ReleaseGitDirty') -or
     -not $PSBoundParameters.ContainsKey('ReleaseGitStatus'))) {
    throw 'Release Git context must include commit, dirty state, and status entries.'
}

$git = Get-Command git -CommandType Application -ErrorAction Stop |
    Select-Object -ExpandProperty Source -First 1
if ($hasReleaseGitContext) {
    $commit = $ReleaseGitCommit.Trim().ToLowerInvariant()
    $dirty = [bool]$ReleaseGitDirty
    $gitStatus = @($ReleaseGitStatus | ForEach-Object { [string]$_ })
    if ($commit -notmatch '^[0-9a-f]{40,64}$') {
        throw 'Release Git context has an invalid commit id.'
    }
    if (-not $dirty -and $gitStatus.Count -ne 0) {
        throw 'Release Git context is clean but contains status entries.'
    }
} else {
    $safeDirectory = $projectRoot.Replace('\','/')
    $gitStatusOutput = @(& $git -c "safe.directory=$safeDirectory" `
        -C $projectRoot status --porcelain --untracked-files=normal 2>&1)
    $gitStatusExitCode = $LASTEXITCODE
    if ($gitStatusExitCode -ne 0) {
        throw ('git status failed while creating package provenance: ' +
            (($gitStatusOutput | ForEach-Object { [string]$_ }) -join '; '))
    }
    $gitStatus = @($gitStatusOutput | ForEach-Object { [string]$_ })
    $dirty = $gitStatus.Count -gt 0
    $commitOutput = @(& $git -c "safe.directory=$safeDirectory" `
        -C $projectRoot rev-parse --verify HEAD 2>&1)
    $commitExitCode = $LASTEXITCODE
    if ($commitExitCode -ne 0 -or $commitOutput.Count -ne 1 -or
        [string]::IsNullOrWhiteSpace([string]$commitOutput[0])) {
        throw ('git rev-parse HEAD failed while creating package provenance: ' +
            (($commitOutput | ForEach-Object { [string]$_ }) -join '; '))
    }
    $commit = ([string]$commitOutput[0]).Trim().ToLowerInvariant()
}
if ($dirty -and -not $AllowDirty) {
    throw 'Worktree is dirty. Use -AllowDirty only for an intentional local candidate package.'
}
$gitStatusSha256 = Get-TextSha256 ($gitStatus -join "`n")

$outputParent = Split-Path -Parent $OutputDirectory
New-Item -ItemType Directory -Force -Path $outputParent | Out-Null
$staging = Join-Path $outputParent ('.ShifRator3.staging-' + [Guid]::NewGuid().ToString('N'))
$previous = $null
New-Item -ItemType Directory -Force -Path (Join-Path $staging 'app') | Out-Null
try {
    Copy-Item -LiteralPath $exe -Destination (Join-Path $staging 'app\myProgram.exe')
    Copy-Item -LiteralPath $runtimeDll -Destination (Join-Path $staging 'app\myCrypto.dll')
    Copy-Item -LiteralPath (Join-Path $templateRoot 'Verify-Package.cmd') -Destination $staging
    Copy-Item -LiteralPath (Join-Path $templateRoot 'Verify-Package.ps1') -Destination $staging
    if (Test-Path -LiteralPath $tutorialRoot -PathType Container) {
        Copy-SafeTree $tutorialRoot (Join-Path $staging 'tutorials')
    }
    [IO.File]::WriteAllText((Join-Path $staging $markerName),
        "Generated by New-ShifRator3Package.ps1`r`n", [Text.UTF8Encoding]::new($false))

    $payloadFiles = @(Get-ChildItem -LiteralPath $staging -Recurse -File | Sort-Object FullName)
    $manifest = [ordered]@{
        formatVersion = 1
        product = $productName
        applicationVersion = $versionInfo.Version
        fileVersion = $versionInfo.FileVersion
        releaseId = $versionInfo.ReleaseId
        releaseFingerprintFormat = $releaseFingerprintFormat
        releaseInputSha256 = $releaseInputSha256
        architecture = 'Win32'
        config = 'Release'
        commit = $commit
        dirty = $dirty
        gitStatus = @($gitStatus)
        gitStatusSha256 = $gitStatusSha256
        createdUtc = (Get-Date).ToUniversalTime().ToString('o')
        entryPoint = 'app/myProgram.exe'
        runtimeDependencies = @('app/myCrypto.dll')
        excluded = @('docs','README.md','BUILD.md','source/build outputs',
            'rsa_keys','system tokens','user archives','send/receive user directories')
        files = @($payloadFiles | ForEach-Object {
            $relative = $_.FullName.Substring($staging.Length + 1).Replace('\','/')
            $role = if ($relative -eq 'app/myProgram.exe') { 'application' }
                elseif ($relative -eq 'app/myCrypto.dll') { 'runtime-dependency' }
                elseif ($relative.StartsWith('tutorials/')) { 'tutorial' }
                elseif ($relative -like 'Verify-Package.*') { 'verifier' }
                else { 'package-marker' }
            [ordered]@{
                path = $relative
                role = $role
                size = $_.Length
                sha256 = Get-Sha256 $_.FullName
            }
        })
    }
    [IO.File]::WriteAllText((Join-Path $staging 'package-manifest.json'),
        ($manifest | ConvertTo-Json -Depth 8), [Text.UTF8Encoding]::new($false))
    $hashFiles = @(Get-ChildItem -LiteralPath $staging -Recurse -File |
        Where-Object Name -ne 'package-checksums.sha256' | Sort-Object FullName)
    $lines = @($hashFiles | ForEach-Object {
        '{0}  {1}' -f (Get-Sha256 $_.FullName),
            $_.FullName.Substring($staging.Length + 1).Replace('\','/')
    })
    [IO.File]::WriteAllLines((Join-Path $staging 'package-checksums.sha256'),
        $lines, [Text.UTF8Encoding]::new($false))

    if ((Get-Sha256 $exe) -ne (Get-Sha256 (Join-Path $staging 'app\myProgram.exe'))) {
        throw 'Packaged EXE differs from the Release EXE.'
    }
    if ((Get-Sha256 $runtimeDll) -ne (Get-Sha256 (Join-Path $staging 'app\myCrypto.dll'))) {
        throw 'Packaged myCrypto.dll differs from the verified runtime input.'
    }

    $stagingVerify = & (Join-Path $staging 'Verify-Package.ps1') -Quiet
    if ($null -eq $stagingVerify -or
        $stagingVerify.Version -ne $versionInfo.Version -or
        $stagingVerify.ReleaseInputSha256 -ne $releaseInputSha256) {
        throw 'Staging verifier did not return the expected structured result.'
    }

    if (Test-Path -LiteralPath $OutputDirectory) {
        if (-not (Test-Path -LiteralPath (Join-Path $OutputDirectory $markerName) -PathType Leaf)) {
            throw "Existing output was not created by this packager: $OutputDirectory"
        }
        $previous = Join-Path $outputParent ('.ShifRator3.previous-' + [Guid]::NewGuid().ToString('N'))
        Move-Item -LiteralPath $OutputDirectory -Destination $previous
    }
    try {
        Move-Item -LiteralPath $staging -Destination $OutputDirectory
        $verifyResult = & (Join-Path $OutputDirectory 'Verify-Package.ps1') -Quiet
        if ($null -eq $verifyResult -or
            $verifyResult.Version -ne $versionInfo.Version -or
            $verifyResult.ReleaseInputSha256 -ne $releaseInputSha256) {
            throw 'Packaged verifier did not return the expected structured result.'
        }
    } catch {
        if (Test-Path -LiteralPath $OutputDirectory -PathType Container) {
            $invalidMarker = Join-Path $OutputDirectory $markerName
            if (Test-Path -LiteralPath $invalidMarker -PathType Leaf) {
                Remove-Item -LiteralPath $OutputDirectory -Recurse -Force
            }
        }
        if ($null -ne $previous -and
            (Test-Path -LiteralPath $previous -PathType Container)) {
            Move-Item -LiteralPath $previous -Destination $OutputDirectory
            $previous = $null
        }
        throw
    }
    if ($null -ne $previous) {
        Remove-Item -LiteralPath $previous -Recurse -Force
        $previous = $null
    }
} finally {
    if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
}

$cloudHelper = & (Join-Path $PSScriptRoot 'Find-WorkspaceReleaseRoot.ps1') `
    -Name 'Publish-ReleasePackageIfConfigured.ps1' -StartPath $projectRoot `
    -Optional
$configuredCloudRoot = $CloudRoot
if ([string]::IsNullOrWhiteSpace($configuredCloudRoot)) { $configuredCloudRoot = $env:WORKSPACE_RELEASE_CLOUD_ROOT }
if ([string]::IsNullOrWhiteSpace($configuredCloudRoot)) {
    $configuredCloudRoot = [Environment]::GetEnvironmentVariable('WORKSPACE_RELEASE_CLOUD_ROOT', 'User')
}
$cloudRequired = -not $SkipCloudPublish -and -not [string]::IsNullOrWhiteSpace($configuredCloudRoot)
$cloudResult = $null
if ($null -ne $cloudHelper) {
    $cloudArgs = @{
        PackageDirectory = $OutputDirectory
        ProductName = $productName
        ReleaseId = $CloudReleaseId
        ZipName = $ZipName
        StartPath = $projectRoot
        SkipCloudPublish = $SkipCloudPublish
    }
    if (-not [string]::IsNullOrWhiteSpace($CloudRoot)) { $cloudArgs.CloudRoot = $CloudRoot }
    $cloudResult = & $cloudHelper @cloudArgs
} elseif ($cloudRequired) {
    throw 'Cloud publish helper not found while CloudRoot is configured.'
} else {
    Write-Host 'Cloud publish helper not found; local package only.'
}
if ($cloudRequired) {
    if ($null -eq $cloudResult -or
        [string]::IsNullOrWhiteSpace([string]$cloudResult.ZipPath) -or
        [string]::IsNullOrWhiteSpace([string]$cloudResult.Sha256Path) -or
        [string]::IsNullOrWhiteSpace([string]$cloudResult.Sha256) -or
        -not (Test-Path -LiteralPath $cloudResult.ZipPath -PathType Leaf) -or
        -not (Test-Path -LiteralPath $cloudResult.Sha256Path -PathType Leaf)) {
        throw 'Cloud publish did not produce the required ZIP and SHA-256 artifacts.'
    }
}

$packagedExe = Join-Path $OutputDirectory 'app\myProgram.exe'
$packagedVersion = [Diagnostics.FileVersionInfo]::GetVersionInfo($packagedExe)
if ($packagedVersion.FileVersion -ne $versionInfo.FileVersion -or
    $packagedVersion.ProductVersion -ne $versionInfo.Version) {
    throw 'Packaged executable PE version does not match app-version.json.'
}

return [pscustomobject]@{
    Product = $productName
    Version = $versionInfo.Version
    FileVersion = $versionInfo.FileVersion
    PackageDirectory = $OutputDirectory
    PackagedExe = $packagedExe
    PackageVerifierPassed = $true
    FilesVerified = $verifyResult.FilesVerified
    SourceCommit = $commit
    Dirty = $dirty
    GitStatusSha256 = $gitStatusSha256
    ReleaseFingerprintFormat = $releaseFingerprintFormat
    ReleaseInputSha256 = $releaseInputSha256
    CloudPublished = $null -ne $cloudResult
    ZipPath = $(if ($null -ne $cloudResult) { $cloudResult.ZipPath } else { $null })
    Sha256Path = $(if ($null -ne $cloudResult) { $cloudResult.Sha256Path } else { $null })
    Sha256 = $(if ($null -ne $cloudResult) { $cloudResult.Sha256 } else { $null })
}
