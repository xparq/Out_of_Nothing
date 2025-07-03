@echo off
::start /min cmd /c C:\sz\SW\cmd\vscode.cmd . %*
start /min cmd /c codium.cmd --user-data-dir=%~dP0tooling\edit\vscodium\user-data-dir  %*
