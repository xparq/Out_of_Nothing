@echo off
setlocal enabledelayedexpansion
call %~dp0tooling\_setenv.cmd

set "pattern=*oon*exe"

if not defined oon_use_exe (
	:: Find the latest exe in the test dir...
	for /f %%f in ('dir /b /o-d /t:w "%SZ_RUN_DIR%\%pattern%" 2^>nul') do (
		set "exe=%%f"
		goto :run
	)
	echo - ERROR: No executable was found matching %pattern%^^!
	goto :eof
) else (
	set exe=...NONE...

	if exist "%SZ_RUN_DIR%/%oon_use_exe%.exe" set exe=%oon_use_exe%.exe
	if exist "%SZ_RUN_DIR%/%oon_use_exe%.cmd" set exe=%oon_use_exe%.cmd
	if exist "%SZ_RUN_DIR%/%oon_use_exe%"     set exe=%oon_use_exe%

	if exist "%SZ_RUN_DIR%/!exe!" goto :run

	echo - ERROR: No %oon_use_exe% or %oon_use_exe%.exe or %oon_use_exe%.cmd in %SZ_RUN_DIR%^^!
	goto :eof
)


:run
echo Latest build: "%SZ_RUN_DIR%\%exe%"
echo Press Ctrl+C now, if this is not the one you wanted^^!
pause
%~dp0test\regression\tc-smoke.cmd %exe% %*
