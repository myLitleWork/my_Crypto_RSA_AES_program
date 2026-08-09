#Requires -Version 5.1
[CmdletBinding()]
param(
    [switch]$BumpPatch,
    [switch]$ForceBumpPatch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$resolveScript = @(
    (Join-Path $project '..\..\TOOLS\scripts\release\Resolve-WorkspaceReleaseScript.ps1'),
    (Join-Path $project '..\..\..\TOOLS\scripts\release\Resolve-WorkspaceReleaseScript.ps1')
) | ForEach-Object { [IO.Path]::GetFullPath($_) } |
    Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
    Select-Object -First 1
if ($null -eq $resolveScript) { throw 'Resolve-WorkspaceReleaseScript.ps1 not found.' }

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
