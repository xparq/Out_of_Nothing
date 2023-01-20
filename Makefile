## Order matters: inference rules first! Also, critically: higher-level ("root") targets first! :-o (?!)
## (At least the .exe needed to precede the .obj blocks; I vaguely remember the docs hinting it doesn't matter!)

EXE=$(out_dir)/$(appname).exe
MODULES=$(out_dir)/main.obj $(out_dir)/world_sfml.obj $(out_dir)/renderer_sfml.obj
INCLUDES=$(src_dir)/*.hpp $(src_dir)/*.h


# NOTE: -showIncludes kinda implies stdout being redirected to capture the list,
#       which also means losing the normal comp. output! :-/
CL_FLAGS=$(CL_FLAGS) -W1 -std:c++latest -MD -EHsc
CL_CMD=cl -nologo -c $(CL_FLAGS) -Fo$(out_dir)/ -Fd$(out_dir)/
LINK_CMD=link -nologo
#!!?? Why does this not do anything useful:
# LINK_CMD=link /LTCG:INCREMENTAL

BUILD_OPT_LABEL=BUILD OPTION:

!if defined(DEBUG)
!MESSAGE
!MESSAGE $(BUILD_OPT_LABEL) DEBUG
CL_CMD=$(CL_CMD) -Zi
!else
!MESSAGE 
!MESSAGE $(BUILD_OPT_LABEL) RELEASE (optimized)
CL_CMD=$(CL_CMD) -O2
!endif

MAKEFILE=$(prjdir)/Makefile

!if defined(SFML_DLL)
!MESSAGE $(BUILD_OPT_LABEL) Linking SFML DLLs
!MESSAGE 
LIBS=	sfml-graphics.lib sfml-window.lib sfml-system.lib \
	sfml-audio.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib
!else
!MESSAGE $(BUILD_OPT_LABEL) Static SFML linkage
!MESSAGE 
CL_CMD=$(CL_CMD) -DSFML_STATIC
LIBS=	sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib \
	sfml-audio-s.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib freetype.lib vorbis.lib vorbisfile.lib \
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
!endif


{$(src_dir)/}.cpp{$(out_dir)/}.obj::
	$(CL_CMD) $<

## This non-batch alternative for attempting to generate .h* deps is futile...
## (Note: redirecting the -showIncludes output with > $*.dep won't work, as $* is 
## illegal in batch rules!)
## Even if the list was massaged into makefile syntax, the MSVC output is still unusable
## for redirection: both the includes AND comp. errors/warnings go to stdout!... :-(
## {$(src_dir)/}.cpp{$(out_dir)/}.obj:
##	$(CL_CMD) $<        -showIncludes > $(out_dir)/$*.hdep


$(EXE):	$(MODULES)
	$(LINK_CMD) /out:$(EXE) $(MODULES) $(LIBS)


## Sorry, no autodep. yet...
$(MODULES): $(INCLUDES) $(MAKEFILE)
