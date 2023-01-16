@echo off

call _setenv.cmd

cl -std:c++20 -EHsc %sz_appname%.cpp -Fo%sz_sfml_test_out%/ -Fe%sz_sfml_test_out%/ ^
	sfml-graphics.lib sfml-window.lib sfml-system.lib ^
	opengl32.lib
