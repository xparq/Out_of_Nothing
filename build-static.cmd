@echo off

call _setenv.cmd

rem	The binary release of the SFML libs is compiled with -MD for importing the MSVC
rem	runtime DLLs, so trying -MT here for a full static link would be futile...
rem	(It would require a complete SFML rebuild with -MT from sources locally.)


@echo on
cl -DSFML_STATIC %sz_CL_FLAGS% %* %sz_src_dir%/*.cpp ^
	-Fo%sz_out_dir%/ -Fe%sz_out_dir%/%sz_appname%.exe -Fd%sz_out_dir%/ -Fd%sz_out_dir%/ ^
	sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib ^
	opengl32.lib freetype.lib vorbis.lib vorbisfile.lib ^
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
@echo off
