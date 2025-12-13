::
:: Build with BusyBox-w32 & MSVC
::
@echo off
call %~dp0tooling/_setenv.cmd
:! Can't use _setenv.sh from a CMD script, even if the rest is basically
:! all SH: those vars can't propagate to Windows, let alone back to SH again. :)

::!!%~dp0tooling/diag/wtime.exe ... <- wtime GETS CONFUSED BY THE QUOTED PARAM BELOW! :-o 

::    busybox sh -c "export SZ_PRJDIR=. && . tooling/_setenv.sh && tooling/build/make -f bbMakefile.msvc %*"
wtime busybox sh -c "export SZ_PRJDIR=. && tooling/build/make -f tooling/build/bbMakefile.msvc %*"

:: The original (BB/sh-scripted) build process with auto-rebuild:
::busybox sh %~dp0tooling/build/_auto-rebuild.sh %*
