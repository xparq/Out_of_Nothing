::
:: Build with NMAKE & MSVC
::
@echo off
call %~dp0tooling/_setenv.cmd

:: The original (BB/sh-scripted) build process with auto-rebuild:
::busybox sh %~dp0tooling/build/_auto-rebuild.sh %*

:: The new (NMAKE-driven, CMD-scripted) process (without auto-rebuild yet!):
%~dp0tooling/diag/wtime.exe nmake /nologo /f nMakefile.msvc %*
