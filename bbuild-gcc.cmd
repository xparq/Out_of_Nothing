::
:: Build with NMAKE & GCC (only tried with w64devkit)
::
@echo off
call %~dp0tooling/_setenv.cmd
:! Can't use _setenv.sh from a CMD script directly, even if the rest is basically
:! all SH: those vars can't propagate to Windows, let alone back to SH again. :)

::!!%~dp0tooling/diag/wtime.exe ... <- wtime GETS CONFUSED BY THE QUOTED PARAM BELOW! :-o 

tooling\busybox sh -c "export SZ_PRJDIR=. && busybox make -f bbMakefile.gcc %*"

::if not errorlevel 1 if not defined GITHUB_ACTION %SZ_TEST_DIR%\regression\tc-smoke
