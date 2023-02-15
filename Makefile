## Order matters! Inference rules first! Also, as NMAKE only processes the first
## root-level tartet if none was named on the command line: higher-level targets
## must be described before dependent target rules!

# Cfg. macros, can be overridden from the make (build) cmdline:
#
#	LINKMODE=static
#	LINKMODE=dll
#
#	DEBUG=1
#	DEBUG=0 (-> release)
#
# Macros set on the MAKE cmdline will override these defaults:
LINKMODE=static
DEBUG=0

# Support my "legacy" env. var names:
prjdir=$(SZ_PRJDIR)
src_dir=$(SZ_SRC_DIR)
out_dir=$(SZ_OUT_DIR)
appname=$(SZ_APPNAME)


!if "$(src_dir)" == "" || "$(out_dir)" == ""
!error - Build env. not initialized (correctly)!
!endif

EXE=$(out_dir)/$(appname).exe

# The existing actual source subdir(s) should/will match the obj. subdir(s):
World_subdir=Model
UI_subdir=UI
#! Note however, about CL /Fo (i.e. "Fuk output subdirs"):
#!      "The specified directory must exist, or the compiler reports error D8003.
#!      The directory isn't created automatically."
#!
#! GCC could do it all right, without a hitch. Just sayin...

MODULES=$(out_dir)/main.obj \
	$(out_dir)/SimApp.obj \
	$(out_dir)/OON.obj \
	$(out_dir)/OON_sfml.obj \
	$(out_dir)/$(World_subdir)/Physics.obj \
	$(out_dir)/$(World_subdir)/Object.obj \
	$(out_dir)/$(World_subdir)/World.obj \
	$(out_dir)/renderer_sfml.obj \
	$(out_dir)/$(UI_subdir)/hud_sfml.obj \
	$(out_dir)/audio_sfml.obj \
	$(out_dir)/TGUI-Clipping.obj \

INCLUDES=$(src_dir)/*.hpp $(src_dir)/*.h \
	$(src_dir)/$(World_subdir)/*.hpp $(src_dir)/$(UI_subdir)/*.hpp $(src_dir)/misc/*.hpp

#CC_FLAGS=$(CC_FLAGS) -nologo
CC_FLAGS=$(CC_FLAGS) -W4 -std:c++latest -MD -EHsc
# For GH #15 (don't rely on manually including cfg.h):
CC_FLAGS=$(CC_FLAGS) -FI cfg.h

CC_OUTDIR_FLAGS_=-Fo$(out_dir)/ -Fd$(out_dir)/
CC_FLAGS_=$(CC_FLAGS) $(CC_OUTDIR_FLAGS_) -c

CC_OUTDIR_FLAGS_World=-Fo$(out_dir)/$(World_subdir)/ -Fd$(out_dir)/$(World_subdir)/
CC_OUTDIR_FLAGS_UI=-Fo$(out_dir)/$(UI_subdir)/ -Fd$(out_dir)/$(UI_subdir)/
CC_FLAGS_World=$(CC_FLAGS) $(CC_OUTDIR_FLAGS_World) -c
CC_FLAGS_UI=$(CC_FLAGS) $(CC_OUTDIR_FLAGS_UI) -c

CC_CMD=cl -nologo
LINK_CMD=link -nologo
#!!?? Why does this not do anything useful:
# LINK_CMD=link /LTCG:INCREMENTAL
# Assuming being called from a script that has already set the path:
BB=busybox
ECHO=@$(BB) echo
MKDIR=$(BB) mkdir -p

MAKEFILE=$(prjdir)/Makefile

BUILD_OPT_LABEL=OPTION:

# Linkmode alternatives:
CC_FLAGS_LINK_static=-DSFML_STATIC
CC_FLAGS_LINK_dll=
#!!NMAKE doesn't seem to support macros in macro names, so we have to dance the !ifs... :-/
#CC_FLAGS_LINKMODE=$(CC_FLAGS_$(CC_FLAGS_LINKMODE))
!if "$(LINKMODE)" == "static"
CC_FLAGS_LINKMODE=$(CC_FLAGS_LINK_static)
!else if "$(LINKMODE)" == "dll"
CC_FLAGS_LINKMODE=$(CC_FLAGS_LINK_dll)
!else
!error Unknown link mode: $(LINKMODE)!
!endif

LIBS_static=\
	sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib \
	sfml-audio-s.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib freetype.lib vorbis.lib vorbisfile.lib \
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
LIBS_dll=\
	sfml-graphics.lib sfml-window.lib sfml-system.lib \
	sfml-audio.lib ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	opengl32.lib
#!!sigh... LIBS=$(LIBS$(LINKMODE))
!if "$(LINKMODE)" == "static"
LIBS=$(LIBS_static)
!else if "$(LINKMODE)" == "dll"
LIBS=$(LIBS_dll)
!else
!error Unknown link mode: $(LINKMODE)!
!endif

# Debug / Release alternatives:
CC_FLAGS_DEBUG_0=-O2 -DNDEBUG
# These -O... below are taken from Dr. Memory's README/Quick start...:
CC_FLAGS_DEBUG_1=-Zi -Oy- -Ob0 -DDEBUG
LINK_FLAGS_DEBUG_0=
LINK_FLAGS_DEBUG_1=-debug
#!!FFS... CC_FLAGS_DEBUG=$(CC_FLAGS_DEBUG_$(DEBUG))
!if defined(DEBUG) && $(DEBUG) == 1
CC_FLAGS_DEBUGMODE=$(CC_FLAGS_DEBUG_1)
LINK_FLAGS_DEBUGMODE=$(LINK_FLAGS_DEBUG_1)
!else if $(DEBUG) == 0
CC_FLAGS_DEBUGMODE=$(CC_FLAGS_DEBUG_0)
LINK_FLAGS_DEBUGMODE=$(LINK_FLAGS_DEBUG_0)
!else
!error Unknown debug mode: $(DEBUG)!
!endif

# File types for the "clean" rule (safety measure against a runaway `rm -rf *`):
CLEANED_OUTPUT_EXT=.exe .obj .pdb .ilk .inc .tmp


#-----------------------------------------------------------------------------
# All Internals below...

CC_FLAGS=$(CC_FLAGS) $(CC_FLAGS_LINKMODE) $(CC_FLAGS_DEBUGMODE)


#-----------------------------------------------------------------------------
# Rules, finally...

{$(src_dir)/}.cpp{$(out_dir)/}.obj::
	$(CC_CMD) $(CC_FLAGS_) $<

{$(src_dir)/$(World_subdir)/}.cpp{$(out_dir)/$(World_subdir)/}.obj::
#!!Alas, this doesn't seem to work in inference rules:
#!!	$(ECHO) SOURCE DRIVE + PATH: %|dpF
	$(CC_CMD) $(CC_FLAGS_World) $<

{$(src_dir)/$(UI_subdir)/}.cpp{$(out_dir)/$(UI_subdir)/}.obj::
#!!Alas, this doesn't seem to work in inference rules:
#!!	$(ECHO) SOURCE DRIVE + PATH: $(%|dpF)<
	$(CC_CMD) $(CC_FLAGS_UI) $<


## This non-batch alternative for attempting to generate .h* deps is futile...
## (Note: redirecting the -showIncludes output with > $*.dep won't work, as $* is 
## illegal in batch rules!)
## But even if the list was massaged into makefile syntax, the MSVC output is still
## "unredirectable", as both the includes AND the errors/warnings go to stdout!... :-(
## {$(src_dir)/}.cpp{$(out_dir)/}.obj:
##	$(CC_CMD) $(CC_FLAGS_) $< -showIncludes > $(out_dir)/$*.hdep


# NMAKE only runs the first root target by default! So...:
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
#!! Make this hamfisted subdir creation less atrocious:
#!! (Not that the rest of the "tree management" is any less lame!)
	@$(MKDIR) $(out_dir)/$(World_subdir)
	@$(MKDIR) $(out_dir)/$(UI_subdir)


DEFAULT:: $(EXE)

#-----------------------------------------------------------------------------
clean:
	$(ECHO)
	$(ECHO) "Cleaning up..."
	$(ECHO)
# FFS... Since migrating the build script to BB sh, the `rm` command started
# failing... It can't find the file for some reason -- the same file that's printed
# I could delete, with the same printed BB command, from the cmdline all right! :-o
# Is it the confusing "stealth" dual globbing behavior again?...
# Not likely: globbing would only be disabled if BB_GLOBBING=0, which shouldn't be
# the case here... Right? RIGHT???
# ...Oh no!... It really is! (Yeah, I did consider that we are running inside BB sh,
# but assumed it would reset it when launching a "foreign" process like CMD... :-o )
	@set BB_GLOBBING=1
	@for %x in ($(CLEANED_OUTPUT_EXT)) do \
		@$(BB) find "$(out_dir)" -type f -name "*%x" -exec $(BB) rm "{}" ^;
#! This didn't work, as `rm -r dir/*x` is too dumb to recurse below dir despite -r
#! (no matter the BB_GLOBBING state):
#		@if exist "$(out_dir)/*%x" \
#			$(BB) rm -r "$(out_dir)/*%x"

#-----------------------------------------------------------------------------
$(EXE):: $(MODULES)
	$(LINK_CMD) $(LINK_FLAGS_DEBUGMODE) /out:$(EXE) $(MODULES) $(LIBS)


## Sorry, no autodep. yet...
$(MODULES): $(INCLUDES) $(MAKEFILE)

## Especially for this one...:
$(out_dir)/main.obj: $(HASH_INCLUDE_FILE)

$(HASH_INCLUDE_FILE):
	$(BB) sh tooling/git/_create_commit_hash_include_file.sh
