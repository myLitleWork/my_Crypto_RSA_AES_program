#Requires -Version 5.1
[CmdletBinding()]
param(
    [switch]$BumpPatch,
    [switch]$ForceBumpPatch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$resolveScript = & (Join-Path $PSScriptRoot 'Find-WorkspaceReleaseRoot.ps1') `
    -Name 'Resolve-WorkspaceReleaseScript.ps1' -StartPath $project

$helper = & $resolveScript -ScriptName 'Sync-VclReleaseVersion.ps1' -StartPath $project
$arguments = @{
    ProjectRoot = $project
    ProductName = 'ShifRator3'
    ResourceFileName = 'ShifRator3Version.rc'
    ContentDirectories = @('tutorials')
    PackageInputPaths = @(
        'deployment\ShifRator3Portable',
        'runtime\myCrypto.dll',
        'myCrypto-bcb.lib'
    )
    VersionHeaderPath = ''
}
if ($BumpPatch) { $arguments.BumpPatch = $true }
if ($ForceBumpPatch) { $arguments.ForceBumpPatch = $true }
return & $helper @arguments
