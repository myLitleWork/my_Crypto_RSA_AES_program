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
return & (Join-Path $PSScriptRoot 'New-ShifRator3Package.ps1') `
    -BuildDirectory $BuildDirectory `
    -AllowDirty
