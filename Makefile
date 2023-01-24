## Order matters! Inference rules first! Also, as NMAKE only processes the first
## root-level tartet if none was named on the command line: higher-level targets
## must be described before dependent target rules!


# Support my "legacy" env. var names:
prjdir=$(SZ_PRJDIR)
src_dir=$(SZ_SRC_DIR)
out_dir=$(SZ_OUT_DIR)
appname=$(SZ_APPNAME)


!if "$(src_dir)" == "" || "$(out_dir)" == ""
!error - Build env. not initialized (correctly)!
!endif

EXE=$(out_dir)/$(appname).exe

MODULES=$(out_dir)/main.obj \
	$(out_dir)/world_sfml.obj \
	$(out_dir)/renderer_sfml.obj \
	$(out_dir)/engine_sfml.obj \
	$(out_dir)/hud_sfml.obj \
	$(out_dir)/audio_sfml.obj \

INCLUDES=$(src_dir)/*.hpp $(src_dir)/*.h

CL_FLAGS=$(CL_FLAGS) -W1 -std:c++latest -MD -EHsc
# For GitHub issue #15 (don't rely on manually including cfg.h):
CL_FLAGS=$(CL_FLAGS) -FI cfg.h
CL_CMD=cl -nologo -c $(CL_FLAGS) -Fo$(out_dir)/ -Fd$(out_dir)/
LINK_CMD=link -nologo
#!!?? Why does this not do anything useful:
# LINK_CMD=link /LTCG:INCREMENTAL
# Assuming being called from a script that has already set the path:
BB=busybox
ECHO=@$(BB) echo

# For the "clean" rule (safety measure against a runaway `rm -rf *`):
CLEANED_OUTPUT_EXT=.exe .obj .pdb .ilk .inc .sh

MAKEFILE=$(prjdir)/Makefile

BUILD_OPT_LABEL=OPTION:


!if defined(DEBUG) && "$(DEBUG)" != "0"
CL_CMD=$(CL_CMD) -Zi -DDEBUG
!else
CL_CMD=$(CL_CMD) -O2 -DNDEBUG
!endif

!if defined(SFML_DLL)
LIBS=	sfml-graphics.lib sfml-window.lib sfml-system.lib \
	sfml-audio.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib
!else
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
## But even if the list was massaged into makefile syntax, the MSVC output is still
## "unredirectable", as both the includes AND the errors/warnings go to stdout!... :-(
## {$(src_dir)/}.cpp{$(out_dir)/}.obj:
##	$(CL_CMD) $<        -showIncludes > $(out_dir)/$*.hdep


# NMAKE only runs the first root target by default! :-o So...:
#-----------------------------------------------------------------------------
DEFAULT:: $(HASH_INCLUDE_FILE)
	$(ECHO) Processing default target ($(EXE))...
!if defined(DEBUG) && "$(DEBUG)" != "0"
	$(ECHO)
	$(ECHO) - $(BUILD_OPT_LABEL) DEBUG
!else
	$(ECHO)
	$(ECHO) - $(BUILD_OPT_LABEL) RELEASE (optimized)
!endif
!if defined(SFML_DLL)
	$(ECHO) - $(BUILD_OPT_LABEL) Link with SFML DLLs
	$(ECHO)
!else
	$(ECHO) - $(BUILD_OPT_LABEL) Static-linked SFML
	$(ECHO)
!endif

DEFAULT:: $(EXE)

#-----------------------------------------------------------------------------
clean:
	$(ECHO)
	$(ECHO) "Cleaning up..."
	$(ECHO)
# FFS... Since migrating the build script to BB sh, the `rm` command started
# failing... It can't find the file for some reason -- the same file that's printed
# I could delete, with the same printed BB command, from the cmdline all right! :-o
# Is it the f* confusing dual globbing behavior again?...
# Not likely: globbing would only be disabled if BB_GLOBBING=0, which shouldn't be
# the case here... Right? RIGHT???
# ...Oh no!... It really is! (Yeah, I did consider that we are running inside BB sh,
# but assumed sh resets it when returning back to CMD!... :-o )
	@set BB_GLOBBING=1
	@for %x in ($(CLEANED_OUTPUT_EXT)) do \
		@if exist "$(out_dir)/*%x" \
			$(BB) rm "$(out_dir)/*%x"

#-----------------------------------------------------------------------------
$(EXE):: $(MODULES)
	$(LINK_CMD) /out:$(EXE) $(MODULES) $(LIBS)


## Sorry, no autodep. yet...
$(MODULES): $(INCLUDES) $(MAKEFILE)

## Espeecially for this one...:
$(out_dir)/main.obj: $(HASH_INCLUDE_FILE)

$(HASH_INCLUDE_FILE):
	$(BB) sh tooling/git/_create_commit_hash_include_file.sh

