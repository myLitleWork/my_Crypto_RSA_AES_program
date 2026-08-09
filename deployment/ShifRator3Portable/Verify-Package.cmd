@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "VERIFY_RC=0"
set "POWERSHELL=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if not exist "%POWERSHELL%" (
  echo ERROR: Windows PowerShell not found: "%POWERSHELL%"
  set "VERIFY_RC=9009"
  goto :finish
)
"%POWERSHELL%" -NoProfile -ExecutionPolicy Bypass -File "%~dp0Verify-Package.ps1"
set "VERIFY_RC=%ERRORLEVEL%"
:finish
endlocal & exit /b %VERIFY_RC%
