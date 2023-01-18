@echo off

call _setenv.cmd

rem	The binary release of the SFML libs is compiled with -MD for importing the MSVC
rem	runtime DLLs, so trying -MT here for a full static link would be futile...
rem	(It would require a complete SFML rebuild with -MT from sources locally.)


cl -DSFML_STATIC -Zi -MD -std:c++latest -EHsc %sz_appname%.cpp -Fo%sz_sfml_test_out%/ -Fe%sz_sfml_test_out%/ -Fd%sz_sfml_test_out%/ ^
	sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib ^
	opengl32.lib freetype.lib vorbis.lib vorbisfile.lib ^
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
