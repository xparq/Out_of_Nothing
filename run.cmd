@echo off
call %~dp0tooling/_setenv.cmd

"%sz_out_dir%/%sz_appname%.exe" %*
rem Launch with a post-mortem persisting console:
rem start cmd /k cd "%sz_prj_dir%" ^& "%sz_out_dir%/%sz_appname%.exe" %*
