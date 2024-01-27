@echo off
setlocal
::call %~dp0../_setenv.cmd keep_sfml_libroot
call %~dp0../_setenv.cmd

pushd %SZ_PRJDIR%
rem !! Amazingly, this started to fail for some unknown reason: that same git cmd
rem !! just works all right outside of that FOR loop... :-o ("usebackq" didn't help).
rem !!for /f %%i in ('git rev-parse --short=8 HEAD') do set last_commit_hash=%%i
rem !! Luckily, this alternative hack still works:
git rev-parse --short=8 HEAD > %sz_tmp_dir%/last-commit.hash
for /f %%i in (%sz_tmp_dir%/last-commit.hash) do set last_commit_hash=%%i
popd

set packname=%SZ_APP_NAME%-%last_commit_hash%.zip
set packfile=%SZ_RELEASE_DIR%/%packname%
rem Some cmds. (like DEL) might fail to see the file without / -> \ conv. :-o
set "packfile=%packfile:/=\%"

rem !! All these below are assumed to be in %SystemRoot%\System32 (see the actual zip command!):
rem !! Also: if the last item had a trailing ^ then CMD would gobble up the next command, too,
rem !! despite the empty line! :-o
set vcredist_dlls=^
	vcruntime140.dll^
	vcruntime140_1.dll^
	msvcp140.dll^
	msvcp140_atomic_wait.dll
	rem !! WTF the _1 gives "zip warning: name not matched" (no matter the ordering/quoting)?! :-o
	rem !! Same for _1d.dll Plain `DIR` just lists it OK, attributes seem fine... :-o
	rem !! -S doesn't help either (accordingly).
	rem !! BUT: if copied locally, it's fine again... :-/ FFS...

set sfml_dlls=openal32.dll

rem Overwrite existing pack?
if exist "%packfile%" (
	echo Deleting existing release pack: "%packfile%"...
	echo Abort now if you want to keep it^!
	pause
	del "%packfile%"
)
if not exist "%SZ_RELEASE_DIR%" md "%SZ_RELEASE_DIR%"

pushd "%SZ_OUT_DIR%"
zip %packfile% ./%SZ_APP_NAME%.exe
popd

pushd "%SZ_SFML_ROOT%/bin"
zip %packfile% ./%sfml_dlls%"
popd

pushd "%SZ_PRJDIR%"
zip -r %packfile% ./%sz_asset_subdir% -x ./%sz_asset_subdir%/music/*.ogg*
zip -r %packfile% ./%sz_asset_subdir%/music/default.ogg

rem Add the VCREDIST DLLs:
rem !! Alas, can't just do as above, as ZIP would want to pack with the full
rem !! paths if vcredist_dlls had full paths... :-/
pushd "%SystemRoot%\System32"
rem !! Amazingly, the shit below was syntactically invalid for CMD with the
rem !! if ... (...) else (...) syntax, no matter the escaping... :-/
if     exist "%packfile%" zip -rS %packfile% %vcredist_dlls%
if not exist "%packfile%" echo. & echo - WARNING: couldn't add the MSVC DLLs ^(%packfile% must be an abs. path^)!
popd

rem Also the `api-ms-win-crt-*` ones! :-o
rem !! This didn't help, and not sure it's a good idea to begin with... E.g. Sysinternals' Listdlls doesn't even mention these.
::pushd "%SystemRoot%\System32"\downlevel
::if     exist "%packfile%" zip -rS %packfile% api-ms-win-crt-*
::if not exist "%packfile%" echo. & echo - WARNING: couldn't add the MSVC DLLs ^(%packfile% must be an abs. path^)!
::popd

popd

set vcruntime140_1=vcruntime140_1.dll
echo. & echo WTF's wrong with %vcruntime140_1%?!... Trying separately...
copy "%SystemRoot%\System32\%vcruntime140_1%" "%sz_tmp_dir%\"
zip -j "%packfile%" "%sz_tmp_dir%\%vcruntime140_1%"

echo.
echo Release pack %packfile% created.

endlocal
