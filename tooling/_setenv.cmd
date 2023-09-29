@echo off
rem	This is expected to be called from a (temporary) process context, where
rem	the env. vars won't persist, and won't clash with anything important!

set sz_appname=oon

rem !!
rem !! These could crash horrendously, if e.g. the existing value has & in it etc...:
rem !!

if "%sz_prjdir%"=="" set sz_prjdir=%~dp0..
if "%sfml_libroot%"=="" set sfml_libroot=%sz_prjdir%/extern/sfml

set sz_src_dir=%sz_prjdir%/src
set sz_asset_subdir=asset
set sz_asset_dir=%sz_prjdir%/%sz_asset_subdir%
set sz_out_dir=%sz_prjdir%/out
set sz_run_dir=%sz_prjdir%/run
set sz_tmp_dir=%sz_prjdir%/tmp
set sz_release_dir=%sz_tmp_dir%/release

set HASH_INCLUDE_FILE=%sz_out_dir%/commit_hash.inc

rem CD to prj root for the rest of the process:
cd "%sz_prjdir%"

if not exist "%sz_out_dir%" md "%sz_out_dir%"

set INCLUDE=%sz_src_dir%;extern/sfw/include;%sfml_libroot%/include;%sz_prjdir%;%INCLUDE%
set LIB=%sfml_libroot%/lib;%LIB%
set PATH=%sz_prjdir%/tooling;%sfml_libroot%/bin;%PATH%
