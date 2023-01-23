@echo off
call %~dp0tooling/_setenv.cmd

busybox sh build.sh %*
