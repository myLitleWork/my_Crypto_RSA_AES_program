@echo off
rem Build ShifRator3 from the command line without starting the IDE.
setlocal EnableExtensions DisableDelayedExpansion
set "BUILD_RC=0"
set "CFG=%~1"
set "MODE=%~2"
set "EXTRA="
set "RSVARS=C:\Program Files (x86)\Embarcadero\Studio\22.0\bin\rsvars.bat"

if "%CFG%"=="" set "CFG=Debug"
if /I "%CFG%"=="Debug" goto :debug
if /I "%CFG%"=="Release" goto :release
echo ERROR: Configuration must be Debug or Release.
set "BUILD_RC=2"
goto :finish

:release
if "%MODE%"=="" (
  call "%~dp0release.bat"
  set "BUILD_RC=%ERRORLEVEL%"
  goto :finish
)
if /I "%MODE%"=="NoLink" (
  echo ERROR: Release NoLink is not supported; use tools\release.bat.
  set "BUILD_RC=2"
  goto :finish
)
echo ERROR: Unknown Release argument: %MODE%
set "BUILD_RC=2"
goto :finish

:debug
if "%MODE%"=="" goto :debug_ready
if /I not "%MODE%"=="NoLink" (
  echo ERROR: Unknown Debug argument: %MODE%
  set "BUILD_RC=2"
  goto :finish
)
set "EXTRA=/p:_SkipLink=true"

:debug_ready
if not exist "%RSVARS%" (
  echo ERROR: rsvars.bat not found: "%RSVARS%"
  set "BUILD_RC=9009"
  goto :finish
)
call "%RSVARS%"
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" goto :finish
pushd "%~dp0.."
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" goto :finish
msbuild myProgram.cbproj /t:Build /p:Config=Debug /p:Platform=Win32 %EXTRA% /v:n /nologo
set "BUILD_RC=%ERRORLEVEL%"
popd

:finish
endlocal & exit /b %BUILD_RC%
