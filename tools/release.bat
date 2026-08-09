@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "BUILD_RC=0"
set "SCRIPT_DIR=%~dp0"
set "POWERSHELL=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
set "RELEASE_ARGS="

if "%~1"=="" goto :arguments_ready
if /I "%~1"=="ForceBump" if "%~2"=="" (
  set "RELEASE_ARGS=-ForceBumpPatch"
  goto :arguments_ready
)
if not "%~1"=="" (
  echo ERROR: optional argument must be ForceBump.
  set "BUILD_RC=2"
  goto :finish
)

:arguments_ready
if not exist "%POWERSHELL%" (
  echo ERROR: Windows PowerShell not found: "%POWERSHELL%"
  set "BUILD_RC=9009"
  goto :finish
)

"%POWERSHELL%" -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%..\scripts\Build-ShifRator3Package.ps1" %RELEASE_ARGS%
set "BUILD_RC=%ERRORLEVEL%"

:finish
endlocal & exit /b %BUILD_RC%
