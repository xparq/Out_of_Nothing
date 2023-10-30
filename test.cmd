@echo off
call %~dp0tooling\_setenv.cmd

:: Max number of main/update loop cycles (-1: no limit):
if "%1" == "" ( set cycles=-1 ) else ( set cycles=%1 )

:: Just run the latest test/oon*.exe, whatever flavor it is...
for /f %%f in ('dir /b /a-d /t:w "%SZ_RUN_DIR%\oon*exe"') do (
	set "latest_exe=%%f"
	goto :break
)
:break

echo Launching: %latest_exe%...
run %latest_exe% ^
	--snd=off --interact --bodies=300 --friction=0.01 --zoom=0.1 --loopcap=%cycles%
