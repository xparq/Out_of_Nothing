@echo off
setlocal enabledelayedexpansion
::
:: If oon_use_exe is not set, just run the latest exe with all the args.
:: Otherwise run "%oon_use_exe" with the args.
::

::!! Even the %1 %2 %3 %4 %5 %6 %7 %8 %9 hacky fallback fails with name=value args! :-o
::!! The = gets replaced with space! :-ooo
::!! So... No support for passing an exe name on the cmdline! :-o
::!! Can only be preset in an env var instead. :-/

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
echo Launching: %exe% --cfg=test/default.cfg %*
 "%SZ_RUN_DIR%\%exe%" --cfg=test/default.cfg %*
