#Requires -Version 5.1
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$syncScript = Join-Path $PSScriptRoot 'Sync-ShifRator3Version.ps1'
$packageScript = Join-Path $PSScriptRoot 'New-ShifRator3Package.ps1'
$bdsRoot = 'C:\Program Files (x86)\Embarcadero\Studio\22.0'
$resolveScript = & (Join-Path $PSScriptRoot 'Find-WorkspaceReleaseRoot.ps1') `
    -Name 'Resolve-WorkspaceReleaseScript.ps1' -StartPath $project

$preflight = & $resolveScript -ScriptName 'Test-VclReleasePrerequisites.ps1' -StartPath $project
$null = & $preflight `
    -ProjectRoot $project `
    -ProjectFilePath (Join-Path $project 'myProgram.cbproj') `
    -SyncScriptPath $syncScript `
    -PackageScriptPath $packageScript `
    -RequiredPackageInputs @(
        'myCrypto-bcb.lib',
        'runtime\myCrypto.dll',
        'deployment\ShifRator3Portable\Verify-Package.cmd',
        'deployment\ShifRator3Portable\Verify-Package.ps1',
        (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-d-x32-1_70.lib'),
        (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-mt-d-x32-1_70.lib'),
        (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-mt-sd-x32-1_70.lib'),
        (Join-Path $bdsRoot 'lib\boost\libboost_serialization-bcb32c-sd-x32-1_70.lib')
    ) `
    -SkipCloudPublish:($env:SHIFRATOR3_SKIP_PORTABLE_PACKAGE -eq '1')

if ($env:SHIFRATOR3_SKIP_VERSION_BUMP -eq '1') {
    Write-Host 'ShifRator3 version bump skipped (SHIFRATOR3_SKIP_VERSION_BUMP=1).'
    $null = & $syncScript
} elseif ($env:SHIFRATOR3_FORCE_VERSION_BUMP -eq '1') {
    $null = & $syncScript -ForceBumpPatch
} else {
    $null = & $syncScript -BumpPatch
}
