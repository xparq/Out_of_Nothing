@echo off
call %~dp0tooling/_setenv.cmd

C:\SW\devel\tool\drmemory\current\bin64\drmemory.exe %sz_out_dir%/%sz_appname%.exe %*
