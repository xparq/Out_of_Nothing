@echo off
setlocal enabledelayedexpansion

set toolchain=mingw

set lib_out_subdir=%toolchain%
set inc_out_subdir=
set "work_dir_slash=%~dp0"

set inc_files=zstd.h

set "bb=%~dp0../../tooling/busybox"

::----------------------------------------------------------------------------
:: No edits needed below.

:: Get the configured ZSTD version from "./VERSION"...
:: Supported values:
:: - Any of the tags, i.e.: 1.5.5 (without the "v" prefix!)
:: - dev (the default branch with the latest commits)
for /f usebackq %%f in ("%work_dir_slash%VERSION") do set ZSTD_VERSION=%%f
if "%ZSTD_VERSION%" == "" (
	echo - ERROR: Couldn't read requested zstd version from "%work_dir_slash%VERSION"^!
	goto :eof
)

:: Set comp. mode...
set linkmode=static

:: Find the repo...
if not "%VERSION%" == "dev" (
	set PACKAGE_FILE=v%ZSTD_VERSION%.zip
	set PACKAGE_URL=https://github.com/facebook/zstd/archive/refs/tags/!PACKAGE_FILE!
) else (
	set PACKAGE_FILE=dev.zip
	set PACKAGE_URL=https://github.com/facebook/zstd/archive/refs/heads/!PACKAGE_FILE!
)

set "repo=%work_dir_slash%zstd-%ZSTD_VERSION%"
if not exist "%repo%/lib" (
	echo - WARNING: Repo dir "%repo%" not found or incompatible^!
	echo Downloading it...

	if not exist "%bb%*" (
		echo - ERROR: BusyBox required, but couldn't be found ^(configured as "%bb%"^)^!
		goto :eof
	)

	if exist "%work_dir_slash%%PACKAGE_FILE%" del %work_dir_slash%%PACKAGE_FILE%
	"%bb%" wget %PACKAGE_URL% -O %work_dir_slash%%PACKAGE_FILE%
	if errorlevel 1 (
		echo - ERROR: Failed to download %PACKAGE_URL%^!
		goto :eof
	)
	"%bb%" unzip -d %work_dir_slash% %work_dir_slash%%PACKAGE_FILE% */lib/*.c */lib/*.h
	if errorlevel 1 (
		echo - ERROR: Couldn't unzip the downloaded repo package?^! :-o
		goto :eof
	)
	if not exist "%repo%/lib" (
		echo - ERROR: Failed to prepare the downloaded repo for building^! :-o
		echo -        No "%repo%/lib"...
		goto :eof
	)
)

:: Otput dirs...
set lib_out_dir=%work_dir_slash%%lib_out_subdir%
set inc_out_dir=%work_dir_slash%%inc_out_subdir%

if not exist "%lib_out_dir%" mkdir "%lib_out_dir%"
if not exist "%lib_out_dir%" (
	echo - ERROR: Couldn't create output dir for the libs: "%lib_out_dir%"^!
	goto :eof
)
if not exist "%inc_out_dir%" mkdir "%inc_out_dir%"
if not exist "%inc_out_dir%" (
	echo - ERROR: Couldn't create output dir for the headers: "%inc_out_dir%"^!
	goto :eof
)


::----------------------------------------------------------------------------
:: Build, finally...

echo.
echo Build mode: %linkmode%
echo.

set "build_out_dir=%repo%/lib/%toolchain%
::goto :cph

if not exist "%build_out_dir%" mkdir "%build_out_dir%"
if not exist "%build_out_dir%" (
	echo - ERROR: Couldn't create build output dir "%build_out_dir%"^!
	goto :eof
)


pushd "%build_out_dir%"
	echo Compiling...
	gcc -c -O2 "%repo%/lib/common/*.c" "%repo%/lib/compress/*.c" "%repo%/lib/decompress/*.c" "-I%repo%/lib:%repo%/lib/decompress:%repo%/lib/compress:%repo%/lib/common"
	echo ...done.
popd


set "lib_file=libzstd.a"
::set "lib=%lib_out_dir%/%lib_file%"

echo Creating lib: %lib_file%...
pushd "%build_out_dir%"
::!! CMD won't expand *.o for ar:
::ar rcs "%lib_file%" *.o
::!! Avoid BB's built-in ar, which can't do -s:
set BB_GLOBBING=1
"%bb%" sh -c "ar.exe rcs %lib_file% *.o"
if not errorlevel 1 move /Y %lib_file% %lib_out_dir%
popd

::cph
echo Copying interface header(s) to %inc_out_dir%...
::!!
::!! WOW, CMD and its fk'd-up cronies can't stop to amaze me! :-o
::!! This failed with the nonsesical "The file cannot be copied onto itself."
::!! (due to a botched check, I suppose): 
::!!
::       copy /Y "%repo%/lib/zstd.h" "%inc_out_dir%\"
::!!WOT??
::!!echo copy /Y "%repo%/lib/zstd.h" "%inc_out_dir%\"
::!!cd
::!! So, doing this workaround instead:
pushd "%repo%\lib"
copy /Y %inc_files% "%inc_out_dir%\"
popd
