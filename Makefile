EXE=$(out_dir)/$(appname).exe
INCLUDES=$(src_dir)/*.hpp $(src_dir)/*.h
CL_CMD=cl -c $(CL_FLAGS) -Fo$@ -Fd$(out_dir)/
LINK_CMD=link
#!!?? Why does this not do anything useful:
# LINK_CMD=link /LTCG:INCREMENTAL

!if defined(DEBUG)
!MESSAGE 
!MESSAGE Preparing DEBUG build...
!MESSAGE 
CL_CMD=$(CL_CMD) -Zi
!else
#!MESSAGE Preparing release build...
!endif

MAKEFILE=$(prjdir)/Makefile

!if defined(SFML_DLL)
!MESSAGE 
!MESSAGE Building with SFML DLLs...
!MESSAGE 
LIBS=	sfml-graphics.lib sfml-window.lib sfml-system.lib \
	sfml-audio.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib
!else
!MESSAGE 
!MESSAGE Preparing static linking with SFML...
!MESSAGE 
CL_CMD=$(CL_CMD) -DSFML_STATIC
LIBS=	sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib \
	sfml-audio-s.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib freetype.lib vorbis.lib vorbisfile.lib \
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
!endif


$(EXE):	$(out_dir)/main.obj $(out_dir)/world_sfml.obj $(out_dir)/renderer_sfml.obj

$(out_dir)/main.obj: $(src_dir)/main.cpp $(INCLUDES) $(MAKEFILE)
	$(CL_CMD) $(src_dir)/main.cpp

$(out_dir)/world_sfml.obj: $(src_dir)/world_sfml.cpp $(INCLUDES) $(MAKEFILE)
	$(CL_CMD) $(src_dir)/world_sfml.cpp

$(out_dir)/renderer_sfml.obj: $(src_dir)/renderer_sfml.cpp $(INCLUDES) $(MAKEFILE)
	$(CL_CMD) $(src_dir)/renderer_sfml.cpp

$(EXE): $(out_dir)/*.obj
	$(LINK_CMD) /out:$(EXE) $(out_dir)/*.obj $(LIBS)
