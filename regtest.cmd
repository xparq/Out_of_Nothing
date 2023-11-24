@echo off
call %~dp0tooling\_setenv.cmd

set "regdir=%SZ_TEST_DIR%\regression"

for /f %%f in ('dir /b /o-d /t:w "%regdir%\*test*cmd"') do (
	echo -------------------------------------------------------------------
	echo TEST CASE: %%f...
	echo -------------------------------------------------------------------
	"%regdir%/%%f"
)
