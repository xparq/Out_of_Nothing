@echo off

set "_edit_dir=%~dP0tooling\edit\vscodium"
set "_edit_cfg_dir=%_edit_dir%\user-data-dir"
set "_edit_cfg_TEMPLATE_dir=%_edit_cfg_dir%.TEMPLATE"

if exist "%_edit_cfg_dir%\user" goto :ok
    xcopy /I /E /-Y "%_edit_cfg_TEMPLATE_dir%\*" "%_edit_cfg_dir%"
if exist "%_edit_cfg_dir%\user" goto :ok
    echo - ERROR: Setting up the editor config seems has failed!
    echo          Check the contents of "%_edit_cfg_dir%"!
goto :eof

:ok
:: The extra shell is to give Code a new shell to crap its log into!
start /min cmd /c codium.cmd --user-data-dir="%_edit_dir%\user-data-dir"  %*
::start /min cmd /c vscode.cmd . %*
