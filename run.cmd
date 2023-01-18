@echo off

call _setenv.cmd

%sz_out_dir%/%sz_appname%.exe %*
