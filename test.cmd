@echo off
call %~dp0tooling\_setenv.cmd

::NOTES:
::
:: * Override any option on the cmdline, as needed (by just repeating)!
:: * We must set --cfg in this setup, as it't not ./default.cfg
:: * --loopcap=-1: no limit

:: Just run the latest test/oon*.exe, whatever flavor it is...
for /f %%f in ('dir /b /a-d /t:w "%SZ_RUN_DIR%\oon*exe"') do (
	set "latest_exe=%%f"
	goto :break
)
:break

echo Launching: %latest_exe%...
echo 	--cfg=test/default.cfg --snd=off --interact --bodies=300 --friction=0.01 --zoom=0.1 --loopcap=-1 %*
"%SZ_RUN_DIR%\%latest_exe%" ^
	--cfg=test/default.cfg --snd=off --interact --bodies=300 --friction=0.01 --zoom=0.1 --loopcap=-1 %*
