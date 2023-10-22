@echo off
:!
:! For _setenv.sh to take effect (those vars can't propagate to the
:! Windows env!...), the entire process must live in the same sh frame!
:!

busybox sh -c "export SZ_PRJDIR=. && . tooling/_setenv.sh && busybox make %*"

:: The original (BB/sh-scripted) build process with auto-rebuild:
::busybox sh %~dp0tooling/build/_auto-rebuild.sh %*
