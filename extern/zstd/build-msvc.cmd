@echo off
setlocal enabledelayedexpansion

set toolchain=msvc

set lib_out_subdir=%toolchain%
set inc_out_subdir=
set "work_dir_slash=%~dp0"

set inc_files=zstd.h

set "bb=%~dp0..\..\tooling\busybox"

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
set linkmode=%1
if "%linkmode%" == "static" (
	set "CLmode=MT"
) else if "%linkmode%" == "dll" (
	set "CLmode=MD"
) else (
	echo Usage: %~n0 static^|dll
	echo Note: this is for the CRT link mode only^!
	echo The zstd lib itself is always build for static linking.
	goto :eof
)

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
echo Build mode: CRT=%linkmode%
echo.

set "build_out_dir=%repo%\lib\%toolchain%\%CLmode%"
::goto :cph

if not exist "%build_out_dir%" mkdir "%build_out_dir%"
if not exist "%build_out_dir%" (
	echo - ERROR: Couldn't create build output dir "%build_out_dir%"^!
	goto :eof
)
pushd "%repo%\lib"
	cl /nologo /c /EHsc /%CLmode% /O2 common/*.c compress/*.c decompress/*.c -I.;decompress;compress;common /Fo%build_out_dir%/ 
popd

set "lib=%lib_out_dir%\zstd-%CLmode%.lib"

echo Creating lib: %lib%...
lib /nologo "/out:%lib%" "%build_out_dir%\*.obj"

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
