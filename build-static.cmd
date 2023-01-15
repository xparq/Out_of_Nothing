@call _setenv.cmd

cl -DSFML_STATIC -MD -std:c++20 -EHsc hello-sfml.cpp -Fo%sz_sfml_test_out%/ -Fe%sz_sfml_test_out%/ ^
	sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib ^
	opengl32.lib freetype.lib vorbis.lib vorbisfile.lib ^
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
