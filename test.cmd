@echo off
call %~dp0tooling\_setenv.cmd

::NOTES:
::
:: * Override any option on the cmdline, as needed! (Repeating overrides.)

setlocal
if _%SZ_APP_NAME%_==__ set SZ_APP_NAME=main

set "pattern=%SZ_TEST_DIR%\*%SZ_APP_NAME%*exe"
:: Just run the latest test/oon*.exe, whatever flavor it is...
for /f %%f in ('dir /b /o-d /t:w "%pattern%"') do (
	set "latest_exe=%%f"
	goto :break
)
:break

if not _%latest_exe%_==__ goto :ok
echo -ERROR: No test exe matching "%pattern%" was found.
goto :eof

:ok
echo Launching: "%SZ_RUN_DIR%\%latest_exe%"
echo   --cfg=test/default.cfg --version --snd=off --bodies=500 --fps-limit=0 --zoom-adjust=0.1 %*
                "%SZ_RUN_DIR%\%latest_exe%" ^
       --cfg=test/default.cfg --version --snd=off --bodies=500 --fps-limit=0 --zoom-adjust=0.1 %*
