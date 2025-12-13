@echo off
call %~dp0..\..\tooling\_setenv.cmd

:: This name is also used to select the corresponding baseline state files:
set tc_name=%~n0

set regdir=%~dp0
set tc_subdir=%tc_name%
set tc_dir=%regdir%%tc_subdir%

set baseline_version=2024-09-18

set "baseline_dir=%regdir%_baseline-%baseline_version%"
set "reference_startstate=%baseline_dir%/1000_bodies-START.state"
set "reference_endstate=%tc_dir%\REFERENCE-END.state"
set "new_endstate=%regdir%\END.state.tmp"

::NOTES:
:: * Override any option on the cmdline, as needed (by repeating)!
:: * We must set --cfg in this setup, as it't not ./default.cfg
:: * --loop-cap=0 means no cycle limit


::set bodies=500
set loop=20

:: Empty means use the latest, otherwise SZ_RUN_DIR/%1:
set oon_use_exe=%1

%SZ_PRJDIR%/tooling/diag/wtime %SZ_PRJDIR%/run-latest ^
--headless ^
--cfg=test/default.cfg ^
--interact ^
--friction=0.01 ^
--zoom-adjust=0.2 ^
--fixed-dt=0.033 ^
--fps-limit=0 ^
--loop-cap=%loop% ^
--exit-on-finish ^
--session=%reference_startstate% ^
--session-save-as=%new_endstate% ^
--no-save-compressed ^
--log-level=D


::busybox diff -b %SZ_RUN_DIR%\RESULT.save %SZ_RUN_DIR%\%baseline_version%-RESULT.ref && echo OK, SAME!

:: The report below weridly only prints `-(` when run from my MinGW CLI shell!... :-o
:: UPDATE: Umm... Watch this...:
::PATH=!666999666!
:: Umm... Er... Well, that wasn't the fix actually (shielding off echo.exe); this was:
setlocal disabledelayedexpansion

fc /b %new_endstate% %reference_endstate% > nul
if errorlevel 1 (
	echo !!! THE RESULTS DIFFER !!! :-(
) else (
	echo OK. ^(Same as of %baseline_version%.^)
)
