::!! Even the %1 %2 %3 %4 %5 %6 %7 %8 %9 hack (instead of shift and %*)
::!! fails with name=value args: the = gets replaced with space! :-ooo
::!! So... This is not not very useful with args on the cmdline. :-/

@echo off
setlocal enabledelayedexpansion

if _%1_ == __ (
	echo Usage: %~n0 exename [args...]
	echo ^(You meant `run-latest` perhaps^?^)
	goto :eof
)

set use_exe=%1

call %~dp0tooling\_setenv.cmd

	set exe=...NONE...

	if exist "%SZ_RUN_DIR%/%use_exe%.exe" set exe=%use_exe%.exe
	if exist "%SZ_RUN_DIR%/%use_exe%.cmd" set exe=%use_exe%.cmd
	if exist "%SZ_RUN_DIR%/%use_exe%"     set exe=%use_exe%

	if exist "%SZ_RUN_DIR%/!exe!" goto :run

	echo - ERROR: No %use_exe% or %use_exe%.exe or %use_exe%.cmd in %SZ_RUN_DIR%^^!
	goto :eof


:run
echo Launching: %exe% --cfg=test/default.cfg %2 %3 %4 %5 %6 %7 %8 %9
 "%SZ_RUN_DIR%\%exe%" --cfg=test/default.cfg %2 %3 %4 %5 %6 %7 %8 %9
