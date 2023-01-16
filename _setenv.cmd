@echo off

set sz_appname=sim-sfml

set _libroot_=%~dp0../current
set INCLUDE=%_libroot_%/include;%INCLUDE%
set LIB=%_libroot_%/lib;%LIB%
set PATH=%_libroot_%/bin;%PATH%

set sz_sfml_test_out=%~dp0out
md %sz_sfml_test_out%

set _libroot_=
