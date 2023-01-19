@echo off
call %~dp0devtool/_setenv.cmd

%sz_out_dir%/%sz_appname%.exe %*
