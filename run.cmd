@echo off
call %~dp0tooling/_setenv.cmd

%sz_out_dir%/%sz_appname%.exe %*
