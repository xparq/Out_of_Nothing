@echo off

rem	This is expected to be called from a temporary context so the
rem	env. vars set here won't persist.

set sz_appname=sim-sfml
set sz_prjdir=%~dp0..

set sfml_libroot=%sz_prjdir%/../../sfml/current

set INCLUDE=%_sfml_libroot_%/include;%INCLUDE%
set LIB=%_sfml_libroot_%/lib;%LIB%
set PATH=%_sfml_libroot_%/bin;%PATH%

set sz_src_dir=%sz_prjdir%/src
set sz_asset_subdir=asset
set sz_asset_dir=%sz_prjdir%/%sz_asset_subdir%
set sz_out_dir=%sz_prjdir%/out
set sz_tmp_dir=%sz_prjdir%/tmp
set sz_release_dir=%sz_tmp_dir%/release

if not exist "%sz_out_dir%" md "%sz_out_dir%"
