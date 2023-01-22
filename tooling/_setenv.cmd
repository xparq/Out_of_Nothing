@echo off

rem	This is expected to be called from a temporary context so the
rem	env. vars set here won't persist.

set sz_appname=sfml-test
set sz_prjdir=%~dp0..

if "%sfml_libroot%"=="" set sfml_libroot=%sz_prjdir%/../../sfml/current

set INCLUDE=%sfml_libroot%/include;%INCLUDE%
set LIB=%sfml_libroot%/lib;%LIB%
set PATH=%sfml_libroot%/bin;%PATH%

set sz_src_dir=%sz_prjdir%/src
set sz_asset_subdir=asset
set sz_asset_dir=%sz_prjdir%/%sz_asset_subdir%
set sz_out_dir=%sz_prjdir%/out
set sz_tmp_dir=%sz_prjdir%/tmp
set sz_release_dir=%sz_tmp_dir%/release

if not exist "%sz_out_dir%" md "%sz_out_dir%"
