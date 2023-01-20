@echo off

set sz_appname=sim-sfml
set sz_prjdir=%~dp0..

set _sfml_libroot_=%sz_prjdir%/../../sfml/current

set INCLUDE=%_sfml_libroot_%/include;%INCLUDE%
set LIB=%_sfml_libroot_%/lib;%LIB%
set PATH=%_sfml_libroot_%/bin;%PATH%

set sz_src_dir=%sz_prjdir%/src
set sz_out_dir=%sz_prjdir%/out
if not exist "%sz_out_dir%" md "%sz_out_dir%"

set _sfml_libroot_=
