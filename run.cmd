@echo off
call %~dp0tooling\_setenv.cmd

if _%~1_ == __ (
	set "exename=%SZ_APP_NAME%-MD.exe"
) else (
	set "exename=%~1"
)

"%SZ_RUN_DIR%\%exename%" %*
rem Launch with a post-mortem persisting console:
rem start cmd /k cd "%sz_prj_dir%" ^& "%sz_out_dir%/%sz_appname%.exe" %*
