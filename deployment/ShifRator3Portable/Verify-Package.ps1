#Requires -Version 5.1
[CmdletBinding()]
param([switch]$Quiet)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$checksumPath = Join-Path $root 'package-checksums.sha256'
$manifestPath = Join-Path $root 'package-manifest.json'

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

function Get-ExpectedRole([string]$RelativePath) {
    $normalized = $RelativePath.Replace('\','/')
    if ($normalized -eq 'app/myProgram.exe') { return 'application' }
    if ($normalized -eq 'app/myCrypto.dll') { return 'runtime-dependency' }
    if ($normalized.StartsWith('tutorials/')) { return 'tutorial' }
    if ($normalized -like 'Verify-Package.*') { return 'verifier' }
    if ($normalized -eq '.shifrator3-release-package') { return 'package-marker' }
    throw "Manifest contains an unexpected payload path: $RelativePath"
}

function Assert-PeWin32([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $reader = [IO.BinaryReader]::new($stream)
    try {
        if ($reader.ReadUInt16() -ne 0x5A4D) { throw "Not an MZ executable: $Path" }
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

foreach ($required in @($checksumPath, $manifestPath, (Join-Path $root 'app\myProgram.exe'),
    (Join-Path $root 'app\myCrypto.dll'))) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required package file is missing: $required"
    }
}

$manifest = Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ($manifest.product -ne 'ShifRator3' -or $manifest.architecture -ne 'Win32' -or
    $manifest.entryPoint -ne 'app/myProgram.exe') {
    throw 'Package manifest identity is invalid.'
}
if ([int]$manifest.releaseFingerprintFormat -lt 1 -or
    [string]$manifest.releaseInputSha256 -notmatch '^[0-9a-f]{64}$') {
    throw 'Package manifest release fingerprint is invalid.'
}
if ([string]$manifest.commit -notmatch '^[0-9a-f]{40,64}$') {
    throw 'Package manifest source commit is invalid.'
}
$manifestGitStatus = @($manifest.gitStatus | ForEach-Object { [string]$_ })
if (-not [bool]$manifest.dirty -and $manifestGitStatus.Count -ne 0) {
    throw 'Package manifest clean state contains Git status entries.'
}
if ([string]$manifest.gitStatusSha256 -ne
    (Get-TextSha256 ($manifestGitStatus -join "`n"))) {
    throw 'Package manifest Git status digest is invalid.'
}
$expectedFileVersion = "$($manifest.applicationVersion).0"
$version = [Diagnostics.FileVersionInfo]::GetVersionInfo((Join-Path $root 'app\myProgram.exe'))
if ($version.FileVersion -ne $expectedFileVersion -or
    $version.ProductVersion -ne $manifest.applicationVersion) {
    throw 'Packaged EXE version does not match package-manifest.json.'
}
Assert-PeWin32 (Join-Path $root 'app\myProgram.exe')
Assert-PeWin32 (Join-Path $root 'app\myCrypto.dll')

$listed = @{}
foreach ($line in Get-Content -LiteralPath $checksumPath -Encoding UTF8) {
    if ($line -notmatch '^([0-9a-f]{64})  ([^\r\n]+)$') {
        throw "Invalid checksum line: $line"
    }
    $relative = $Matches[2].Replace('/', '\')
    if ([IO.Path]::IsPathRooted($relative) -or $relative.Split('\') -contains '..') {
        throw "Unsafe checksum path: $relative"
    }
    $key = $relative.ToLowerInvariant()
    if ($listed.ContainsKey($key)) { throw "Duplicate checksum path: $relative" }
    $path = [IO.Path]::GetFullPath((Join-Path $root $relative))
    if (-not $path.StartsWith($root + [IO.Path]::DirectorySeparatorChar,
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Checksum path escapes package: $relative"
    }
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Package file is missing: $relative"
    }
    if ((Get-Sha256 $path) -ne $Matches[1]) { throw "Checksum mismatch: $relative" }
    $listed[$key] = $true
}

$actualFiles = @(Get-ChildItem -LiteralPath $root -Recurse -File |
    Where-Object Name -ne 'package-checksums.sha256')
foreach ($file in $actualFiles) {
    if (($file.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw "Reparse point is forbidden in package: $($file.FullName)"
    }
    $relative = $file.FullName.Substring($root.Length + 1).ToLowerInvariant()
    if (-not $listed.ContainsKey($relative)) { throw "File is absent from checksums: $relative" }
}
if ($listed.Count -ne $actualFiles.Count) { throw 'Checksum set does not match package files.' }

$manifestFiles = @{}
foreach ($entry in $manifest.files) {
    $relative = ([string]$entry.path).Replace('/', '\')
    if ([string]::IsNullOrWhiteSpace($relative) -or
        [IO.Path]::IsPathRooted($relative) -or
        $relative.Split('\') -contains '..') {
        throw "Unsafe manifest path: $relative"
    }
    $key = $relative.ToLowerInvariant()
    if ($manifestFiles.ContainsKey($key)) { throw "Duplicate manifest path: $key" }
    $path = [IO.Path]::GetFullPath((Join-Path $root $relative))
    if (-not $path.StartsWith($root + [IO.Path]::DirectorySeparatorChar,
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Manifest path escapes package: $relative"
    }
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Manifest file is missing: $relative"
    }
    if ([Int64]$entry.size -ne (Get-Item -LiteralPath $path).Length) {
        throw "Manifest size mismatch: $relative"
    }
    if ([string]$entry.sha256 -notmatch '^[0-9a-f]{64}$' -or
        [string]$entry.sha256 -ne (Get-Sha256 $path)) {
        throw "Manifest SHA-256 mismatch: $relative"
    }
    $expectedRole = Get-ExpectedRole $relative
    if ([string]$entry.role -ne $expectedRole) {
        throw "Manifest role mismatch: $relative"
    }
    $manifestFiles[$key] = $true
}
$manifestPayloadFiles = @($actualFiles |
    Where-Object Name -ne 'package-manifest.json')
foreach ($file in $manifestPayloadFiles) {
    $relative = $file.FullName.Substring($root.Length + 1).ToLowerInvariant()
    if (-not $manifestFiles.ContainsKey($relative)) { throw "File is absent from manifest: $relative" }
}
if ($manifestFiles.Count -ne $manifestPayloadFiles.Count) {
    throw 'Manifest file set does not match package payload files.'
}

if (-not $Quiet) { Write-Host 'ShifRator3 package verification passed.' }
return [pscustomobject]@{
    Product = 'ShifRator3'
    Version = [string]$manifest.applicationVersion
    FileVersion = $version.FileVersion
    FilesVerified = $listed.Count
    ReleaseFingerprintFormat = [int]$manifest.releaseFingerprintFormat
    ReleaseInputSha256 = [string]$manifest.releaseInputSha256
}
