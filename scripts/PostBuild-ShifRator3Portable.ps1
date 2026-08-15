#Requires -Version 5.1
[CmdletBinding()]
param([string]$BuildDirectory)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if ($env:SHIFRATOR3_SKIP_PORTABLE_PACKAGE -eq '1') { return }
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
    $BuildDirectory = Join-Path $project 'Win32\Release'
}
$package = & (Join-Path $PSScriptRoot 'New-ShifRator3Package.ps1') `
    -BuildDirectory $BuildDirectory `
    -AllowDirty
$commit = & (Join-Path $PSScriptRoot 'Find-WorkspaceReleaseRoot.ps1') `
    -Name 'Commit-VclReleaseVersion.ps1' -StartPath $project
$null = & $commit -ProjectRoot $project -ProductName 'ShifRator3'
return $package
