::
:: Build with NMAKE & MSVC
::
@echo off
call %~dp0tooling/_setenv.cmd

:: The original (BB/sh-scripted) build process with auto-rebuild:
::busybox sh %~dp0tooling/build/_auto-rebuild.sh %*


:: #557:
echo.
echo ****** WARNING:
echo - This build procedure doesn't support header dependency tracking!
echo - It also FAILS on source trees that have the same source filename
echo   across multiple dirs!
echo ***************
echo.


:: The new (NMAKE-driven, CMD-scripted) process (without auto-rebuild yet!):
%~dp0tooling/diag/wtime.exe nmake /nologo /f tooling\build\nMakefile.msvc %*

if not errorlevel 1 if not defined GITHUB_ACTION %SZ_TEST_DIR%\regression\tc-smoke
