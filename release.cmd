@echo off
setlocal
call %~dp0tooling/_setenv.cmd keep_sfml_libroot

pushd %sz_prjdir%
for /f %%i in ('git rev-parse --short HEAD') do set last_commit_hash=%%i
popd

set packname=%sz_appname%-%last_commit_hash%.zip
set packfile=%sz_release_dir%/%packname%
rem Some cmds. (like DEL) might fail to see the file without / -> \ conv. :-o
set "packfile=%packfile:/=\%"

set sfml_dlls=openal32.dll

if exist "%packfile%" (
	echo Deleting existing release pack: "%packfile%"...
	echo Abort now if you want to keep it^!
	pause
	del "%packfile%"
)
if not exist "%sz_release_dir%" md "%sz_release_dir%"


pushd "%sz_out_dir%"
zip %packfile% ./%sz_appname%.exe
popd

pushd "%sfml_libroot%/bin"
zip %packfile% ./%sfml_dlls%"
popd

pushd "%sz_prjdir%"
rem zip -r %packfile% ./%sz_asset_subdir% -x ./%sz_asset_subdir%/music/*.ogg*
zip -r %packfile% ./%sz_asset_subdir% -x ./%sz_asset_subdir%/music/*.ogg*
zip -r %packfile% ./%sz_asset_subdir%/music/default.ogg
popd

echo.
echo Release pack %packfile% created.

endlocal
