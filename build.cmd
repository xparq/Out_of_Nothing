@echo off

call _setenv.cmd

@echo on
cl %sz_CL_FLAGS% %* %sz_src_dir%/%sz_appname%.cpp -Fo%sz_out_dir%/ -Fe%sz_out_dir%/ -Fd%sz_out_dir%/ ^
	sfml-graphics.lib sfml-window.lib sfml-system.lib ^
	opengl32.lib
@echo off
