@echo off
call %~dp0..\..\tooling\_setenv.cmd

:: This name is also used to select the corresponding baseline state files:
set tc_name=%~n0

set regdir=%~dp0
set tc_subdir=%tc_name%
set tc_dir=%regdir%%tc_subdir%

set baseline_version=ea39db36

set "baseline_dir=%regdir%_baseline-%baseline_version%"
set "reference_startstate=%baseline_dir%/500_bodies-START.state"
set "reference_endstate=%tc_dir%\REFERENCE-END.state"
set "new_endstate=%regdir%\END.state.tmp"

::NOTES:
:: * Override any option on the cmdline, as needed (by repeating)!
:: * We must set --cfg in this setup, as it't not ./default.cfg
:: * --loop-cap=0 means no cycle limit


::set bodies=500
set loop=500

:: Empty means use the latest:
set oon_use_exe=%1

%SZ_PRJDIR%/tooling/diag/wtime %SZ_PRJDIR%/run-latest ^
--headless ^
--cfg=test/default.cfg --snd=off --interact ^
--friction=0.01 ^
--zoom-adjust=0.2 ^
--fixed-dt=0.033 ^
--fps-limit=0 ^
--loop-cap=%loop% ^
--exit-on-finish ^
--session=%reference_startstate% ^
--session-save-as=%new_endstate% ^


::busybox diff -b %SZ_RUN_DIR%\RESULT.save %SZ_RUN_DIR%\%baseline_version%-RESULT.ref && echo OK, SAME!
fc /b %new_endstate% %reference_endstate% > nul
if errorlevel 2 (
	echo - ERROR: File Compare failed. Result file was not created?
	goto :eof
)

if errorlevel 1 (
	echo !!! THE RESULTS DIFFER !!! :-(
) else (
	echo OK. ^(Same as of %baseline_version%.^)
)
