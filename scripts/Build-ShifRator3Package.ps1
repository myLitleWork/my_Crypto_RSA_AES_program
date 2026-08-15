#Requires -Version 5.1
[CmdletBinding()]
param(
    [switch]$AllowDirty,
    [switch]$SkipVersionBump,
    [switch]$ForceBumpPatch,
    [switch]$SkipCloudPublish,
    [string]$CloudRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
if ($env:SHIFRATOR3_SKIP_VERSION_BUMP -eq '1') { $SkipVersionBump = $true }
if ($env:SHIFRATOR3_FORCE_VERSION_BUMP -eq '1') { $ForceBumpPatch = $true }
if ($SkipVersionBump -and $ForceBumpPatch) {
    throw 'Use either -SkipVersionBump or -ForceBumpPatch, not both.'
}

$resolve = & (Join-Path $PSScriptRoot 'Find-WorkspaceReleaseRoot.ps1') `
    -Name 'Resolve-WorkspaceReleaseScript.ps1' -StartPath $project

$syncScript = Join-Path $PSScriptRoot 'Sync-ShifRator3Version.ps1'
$packageScript = Join-Path $PSScriptRoot 'New-ShifRator3Package.ps1'
$bdsRoot = 'C:\Program Files (x86)\Embarcadero\Studio\22.0'
$requiredInputs = @(
    'myCrypto-bcb.lib',
    'runtime\myCrypto.dll',
    'deployment\ShifRator3Portable\Verify-Package.cmd',
    'deployment\ShifRator3Portable\Verify-Package.ps1',
    (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-d-x32-1_70.lib'),
    (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-mt-d-x32-1_70.lib'),
    (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-mt-sd-x32-1_70.lib'),
    (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-sd-x32-1_70.lib')
)
$preflight = & $resolve -ScriptName 'Test-VclReleasePrerequisites.ps1' -StartPath $project
$preflightArgs = @{
    ProjectRoot = $project
    ProjectFilePath = (Join-Path $project 'myProgram.cbproj')
    SyncScriptPath = $syncScript
    PackageScriptPath = $packageScript
    RequiredPackageInputs = $requiredInputs
    SkipCloudPublish = $SkipCloudPublish
}
if (-not [string]::IsNullOrWhiteSpace($CloudRoot)) { $preflightArgs.CloudRoot = $CloudRoot }
$null = & $preflight @preflightArgs

# Parse every project-local PowerShell release input with Windows PowerShell 5.1
# before the common helper can acquire the mutex and mutate release state.
$parseErrors = [Collections.Generic.List[object]]::new()
$psScripts = @(
    Get-ChildItem -LiteralPath (Join-Path $project 'scripts') -File -Filter '*.ps1'
    Get-ChildItem -LiteralPath (Join-Path $project 'deployment') -Recurse -File -Filter '*.ps1'
)
foreach ($script in $psScripts) {
    $tokens = $null
    $errors = $null
    $null = [Management.Automation.Language.Parser]::ParseFile(
        $script.FullName,
        [ref]$tokens,
        [ref]$errors
    )
    foreach ($error in $errors) { $parseErrors.Add($error) }
}
if ($parseErrors.Count -gt 0) {
    throw ('PowerShell parser check failed: ' +
        (($parseErrors | ForEach-Object { $_.Message }) -join '; '))
}

$helper = & $resolve -ScriptName 'Invoke-VclReleasePackageBuild.ps1' -StartPath $project
$arguments = @{
    ProjectRoot = $project
    ProjectFileName = 'myProgram.cbproj'
    ProductName = 'ShifRator3'
    ExecutableName = 'myProgram.exe'
    PackagedExecutableRelativePath = 'app\myProgram.exe'
    SyncScriptPath = $syncScript
    PackageScriptPath = $packageScript
    RequiredPackageInputs = $requiredInputs
    AllowDirty = $AllowDirty
    SkipVersionBump = $SkipVersionBump
    ForceBumpPatch = $ForceBumpPatch
    SkipCloudPublish = $SkipCloudPublish
    PassReleaseGitContext = $true
    RequireReleaseFingerprint = $true
}
if (-not [string]::IsNullOrWhiteSpace($CloudRoot)) { $arguments.CloudRoot = $CloudRoot }
return & $helper @arguments
