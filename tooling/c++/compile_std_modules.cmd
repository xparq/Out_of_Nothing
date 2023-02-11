rem Check the flags in the makefile to match them here!
rem
rem (Initially those were set by _setenv, which I then reverted,
rem can't remember why exactly, but this shows that centralizing
rem some of it wasn't a bad idea after all.)

cl /std:c++latest /EHsc /nologo /W4 /MD /c "%VCToolsInstallDir%\modules\std.ixx"
