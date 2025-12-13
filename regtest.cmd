@echo off

setlocal enabledelayedexpansion

call %~dp0tooling\_setenv.cmd

set "_testdir=%SZ_TEST_DIR%\regression"
::pushd "%_testdir%"
for /f %%f in ('dir /b /o-d /t:w "%_testdir%\tc*cmd"') do (
	echo -------------------------------------------------------------------
	echo TEST CASE: %%~nf...
	echo -------------------------------------------------------------------

	setlocal
	rem Not `call`ing caused weird subshell-execution mishaps for CMD...:
	call "%_testdir%\%%f"
	endlocal
)
::popd
goto :eof
