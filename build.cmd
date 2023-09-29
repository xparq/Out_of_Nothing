@echo off
call %~dp0tooling/_setenv.cmd

busybox sh %~dp0tooling/build/_build.sh %*
