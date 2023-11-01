@echo off
call %~dp0tooling\_setenv.cmd

:: Max number of main/update loop cycles (-1: no limit):
if "%1" == "" (
	set loopcap=--loopcap=-1
) else (
	set loopcap=--loopcap=%1
	shift
)

:: Just run the latest test/oon*.exe, whatever flavor it is...
for /f %%f in ('dir /b /a-d /t:w "%SZ_RUN_DIR%\oon*exe"') do (
	set "latest_exe=%%f"
	goto :break
)
:break

echo Launching: %latest_exe%...
echo 	--cfg=test/default.cfg --snd=off --interact --bodies=300 --friction=0.01 --zoom=0.1 %loopcap% %1 %2 %3 %4 %5

:: Must set the cfg file in this setup, as it't not ./default.cfg:
run %latest_exe% ^
	--cfg=test/default.cfg --snd=off --interact --bodies=300 --friction=0.01 --zoom=0.1 %loopcap% %1 %2 %3 %4 %5
