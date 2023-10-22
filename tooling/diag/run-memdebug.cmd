@echo off
call %~dp0tooling/_setenv.cmd

C:\SW\devel\tool\drmemory\current\bin64\drmemory.exe %SZ_OUT_DIR%/%SZ_APPNAME%.exe %*
