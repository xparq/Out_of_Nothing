@echo off
call %~dp0tooling\_setenv.cmd

::NOTES:
::
:: %1: config name tag, i.e. basename of a test/*.cfg.cmd file, without all the ext.

:: Just run the latest test/oon*.exe, whatever flavor it is...
for /f %%f in ('dir /b /a-d /t:w "%SZ_RUN_DIR%\oon*exe"') do (
	set "latest_exe=%%f"
	goto :break
)
:break

echo Launching: %latest_exe%...
call test\%1.cfg.cmd "%SZ_RUN_DIR%\%latest_exe%"
