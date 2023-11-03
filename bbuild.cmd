@echo off
call %~dp0tooling/_setenv.cmd
:! Can't use _setenv.sh from a CMD script, even if the rest is basically
:! all SH: those vars can't propagate to Windows, let alone back to SH again. :)

busybox sh -c "export SZ_PRJDIR=. && . tooling/_setenv.sh && busybox make %*"

:: The original (BB/sh-scripted) build process with auto-rebuild:
::busybox sh %~dp0tooling/build/_auto-rebuild.sh %*
