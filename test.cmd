@echo off
call %~dp0tooling\_setenv.cmd

::NOTES:
::
:: * Override any option on the cmdline, as needed! (Repeating overrides.)

setlocal
::!! It's set, but the build doesn't yet use that name, so that and this
::!! won't agree!... Need to hardcode what the build currently does:
::if _%SZ_APP_NAME%_==__ set SZ_APP_NAME=main
set "pattern=*%main%*exe"
if not "%1" == "" (
	set "pattern=*%1*"
)


:: Just run the latest test/oon*.exe, whatever flavor it is...
for /f %%f in ('dir /b /o-d /t:w "%SZ_TEST_DIR%\%pattern%"') do (
	set "latest_exe=%%f"
	goto :break
)
:break

if not _%latest_exe%_==__ goto :ok
echo -ERROR: No test exe matching "%SZ_TEST_DIR%\%pattern%" was found.
goto :eof

:ok
echo Launching: "%SZ_TEST_DIR%\%latest_exe%"
echo   --cfg=test/default.cfg --version --snd=off --bodies=500 --fps-limit=0 --zoom-adjust=0.1 %*
                "%SZ_TEST_DIR%\%latest_exe%" ^
       --cfg=test/default.cfg --version --snd=off --bodies=500 --fps-limit=0 --zoom-adjust=0.1 %*
