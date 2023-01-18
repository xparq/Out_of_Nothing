@echo off

set sz_appname=sim-sfml

set _sfml_libroot_=%~dp0../current
set INCLUDE=%_sfml_libroot_%/include;%INCLUDE%
set LIB=%_sfml_libroot_%/lib;%LIB%
set PATH=%_sfml_libroot_%/bin;%PATH%

set sz_src_dir=%~dp0src
set sz_out_dir=%~dp0out
if not exist "%sz_out_dir%" md "%sz_out_dir%"

set sz_CL_FLAGS=-Zi -std:c++latest -MD -EHsc 

set _sfml_libroot_=
