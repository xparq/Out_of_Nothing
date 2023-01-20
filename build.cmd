@echo off
call %~dp0tooling/_setenv.cmd

::rem -- Why the hell aren't the env vars propagated automatically?!
nmake /nologo %* "prjdir=%sz_prjdir%" "src_dir=%sz_src_dir%" "out_dir=%sz_out_dir%" "appname=%sz_appname%" "INCLUDE=%INCLUDE%" "LIB=%LIB%"
