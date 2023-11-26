@echo off
call %~dp0tooling\_setenv.cmd

set "regdir=%SZ_TEST_DIR%\regression"

for /f %%f in ('dir /b /o-d /t:w "%regdir%\tc*cmd"') do (
	echo -------------------------------------------------------------------
	echo TEST CASE: %%~nf...
	echo -------------------------------------------------------------------

	rem Not `call`ing caused weird subshell-execution mishaps for CMD...:
	call "%regdir%/%%f"
)
