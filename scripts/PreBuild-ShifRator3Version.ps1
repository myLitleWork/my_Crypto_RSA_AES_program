#Requires -Version 5.1
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$syncScript = Join-Path $PSScriptRoot 'Sync-ShifRator3Version.ps1'
$packageScript = Join-Path $PSScriptRoot 'New-ShifRator3Package.ps1'
$bdsRoot = 'C:\Program Files (x86)\Embarcadero\Studio\22.0'
$resolveScript = @(
    (Join-Path $project '..\..\TOOLS\scripts\release\Resolve-WorkspaceReleaseScript.ps1'),
    (Join-Path $project '..\..\..\TOOLS\scripts\release\Resolve-WorkspaceReleaseScript.ps1')
) | ForEach-Object { [IO.Path]::GetFullPath($_) } |
    Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
    Select-Object -First 1
if ($null -eq $resolveScript) { throw 'Resolve-WorkspaceReleaseScript.ps1 not found.' }

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
