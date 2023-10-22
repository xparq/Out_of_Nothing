#:
#! NOTE: The MAKE process must be run in the project (root) dir,
#!       because everything is assumed to be relative to that!
#!	 (BB's pdpmake doesn't support abs. paths for targets! :-o )
#!

#--- BUILD MODE DEFAULTS -----------------------------------------------------
DEBUG = 0
CRT = dll
#	CRT=dll is always the case with the pre-built SFML libs, so not use changing it!
SFML = static

#--- PRJ DEFINITIONS ---------------------------------------------------------
SZ_APP_NAME ?= test
SZ_PRJ_DIR  ?= .
SZ_SRC_SUBDIR ?= src
SZ_OUT_SUBDIR ?= build.out
SZ_RUN_SUBDIR ?= .

exe = $(exedir)/$(SZ_APP_NAME).exe
srcdir = $(SZ_SRC_SUBDIR)
outdir = $(SZ_OUT_SUBDIR)
objdir = $(SZ_OUT_SUBDIR)
ifcdir = $(objdir)/$(SZ_SRC_SUBDIR)
exedir = $(SZ_RUN_SUBDIR)

SOURCES != busybox sh -c "find $(srcdir) -name '*.c' -o -name '*.cpp' -o -name '*.ixx'"
#! No need for this hackery, since the entire src tree is replicated in $OUT!...
#!!?? Can't get the BB find syntax right this way:
#!!??SOURCES != find $(srcdir) -not \( -name 'tmp' -prune \) ...
#SOURCES != busybox sh -c "find $(srcdir) -not \( -name $(SZ_OUT_SUBDIR) -prune \) -a \( -name '*.c' -o -name '*.cpp' -o -name '*.ixx' \)"
#test:: ; @echo SOURCES: $(SOURCES)

#!! Kludge until I find out the correct way to support C++ modules...
#!! Also, any module change will trigger full rebuild, coz no sane way
#!! to auto-track actual deps yet (even less than headers)! :-/
#!! (E.g. https://stackoverflow.com/questions/69190433/auto-generate-dependencies-for-modules)
CPP_MODULE_IFCS = $(ifcdir)/*.ifc

EXT_LIBS =\
	$(SZ_SFML_LIBROOT)/lib/$(sfml_and_deps_libs) \
	extern/sfw/lib/msvc/sfw$(cflags_crt_linkmode)$(buildmode_debug_file_tag).lib \
	extern/zstd/msvc/zstd$(cflags_crt_linkmode).lib \


# Adapt the various flavors of the SFML libs (as per the debug/link modes)...
SFML_linkmode=$(SFML)

sfml_libs__static = sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib sfml-audio-s.lib
sfml_libs__dll    = sfml-graphics.lib   sfml-window.lib   sfml-system.lib   sfml-audio.lib
sfml_libs = $(sfml_libs__$(SFML_linkmode))

sfml_and_deps_libs__static = $(sfml_libs) opengl32.lib freetype.lib\
	ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
sfml_and_deps_libs__dll = $(sfml_libs) opengl32.lib
sfml_and_deps_libs = $(sfml_and_deps_libs__$(SFML_linkmode))

#debug:; @echo sfml_libs: $(sfml_libs)

#----------------------------
# Static/DLL CRT link mode
#------
# Keep these -... not /... because they're used for (lib) file tagging, too! :)

_cflags_crt_linkmode__static := -MT
_cflags_crt_linkmode__dll    := -MD
cflags_crt_linkmode := $(_cflags_crt_linkmode__$(CRT))

_cflags_crt_linkmode_with_debug__0 = $(cflags_crt_linkmode)
_cflags_crt_linkmode_with_debug__1 = $(cflags_crt_linkmode)d
cflags_crt_linkmode_with_debug = $(_cflags_crt_linkmode_with_debug__$(DEBUG))

#----------------------
# DEBUG/RELEASE mode
#------
_cflags_debug__0 = -DNDEBUG
_cflags_debug__1 = /DDEBUG /ZI /Od /Oy- /Ob0 /RTCsu /Fd$(objdir)/
#	The -O...s above are borrowed from Dr. Memory's README/Quick start.
#	-ZI enables edit-and-continue (but it only exists for Intel CPUs!).
cflags_debug = $(_cflags_debug__$(DEBUG))

_linkflags_debug__0 =
_linkflags_debug__1 = -debug -incremental -editandcontinue -ignore:4099
linkflags_debug = $(_linkflags_debug__$(DEBUG))

#------------------------------
# SFML link mode (static/dll)
#------
_cflags_sfml_linkmode__static = -DSFML_STATIC
cflags_sfml_linkmode = $(_cflags_sfml_linkmode__$(SFML))


# Output dir/file tags for build alternatives
# NOTE: Must differ from their "no ..." counterparts to avoid code (linking) mismatches!
#!Sz: CRT=dll is always the case with the pre-built SFML libs, so not use changing it! (See also at CRT=...)
_buildmode_crtdll_dir_tag__static  =
_buildmode_crtdll_dir_tag__dll     = .crtDLL
buildmode_crtdll_dir_tag           = $(_buildmode_crtdll_dir_tag__$(CRT))
_buildmode_crtdll_file_tag__static =
_buildmode_crtdll_file_tag__dll    = -crtDLL
buildmode_crtdll_file_tag          = $(_buildmode_crtdll_file_tag__$(CRT))

_buildmode_debug_dir_tag__0  =
_buildmode_debug_dir_tag__1  = .DEBUG
buildmode_debug_dir_tag      = $(_buildmode_debug_dir_tag__$(DEBUG))
_buildmode_debug_file_tag__0 =
_buildmode_debug_file_tag__1 = -d
buildmode_debug_file_tag     = $(_buildmode_debug_file_tag__$(DEBUG))

buildmode_sfml_linkmode_dir_tag = .sfml-$(SFML_linkmode)

# All combined:
buildmode_dir_tag  =  $(buildmode_crtdll_dir_tag)$(buildmode_sfml_linkmode_dir_tag)$(buildmode_debug_dir_tag)
buildmode_file_tag = $(buildmode_crtdll_file_tag)$(buildmode_sfml_linkmode_file_tag)$(buildmode_debug_file_tag)
#debug:; @echo buildmode_dir_tag: $(buildmode_dir_tag)
#debug:; @echo buildmode_file_tag: $(buildmode_file_tag)

# This must come after all the shakes above (for the mandatory := here):
sfml_libs := $(sfml_libs:%.lib=%$(buildmode_debug_file_tag).lib)
#debug:; @echo sfml_libs: $(sfml_libs)

#--- CUSTOMIZATIONS ----------------------------------------------------------
CL   := cl /nologo
CC   := $(CL) -c
CCC  := $(CL) -c /EHsc
LINK := link.exe /nologo
SH   := busybox sh


# Not using += because we're prepending! (To support overriding!)
CFLAGS := # Get dir of the default /O1 (but still allow overrides from the CMDLINE!)
CFLAGS := /I$(objdir) $(cflags_crt_linkmode_with_debug) $(cflags_debug) $(CFLAGS)
CFLAGS := /O2 $(CFLAGS) 
CPPFLAGS := /std:c++latest /ifcOutput $(ifcdir)/ /ifcSearchDir $(ifcdir)/ $(CPPFLAGS)
CPPFLAGS := $(cflags_sfml_linkmode) $(CPPFLAGS)
LINKFLAGS := $(linkflags_debug) $(LINKFLAGS)


#-----------------------------------------------------------------------------
# BUILD "ENGINE"
#-----------------------------------------------------------------------------

# No need for such pattern shenanigans, as the current dir (./) seems to
# have been appended by `find` all right! :)
#!!_src_prep := FPFX_PLACEHOLDER_$(SOURCES:%.c=FPFX_PLACEHOLDER_%.c)
objs := $(SOURCES:%=$(outdir)/%)
#test:: ; @echo objs: $(objs)
objs := $(objs:%.c=%.obj)
objs := $(objs:%.cpp=%.obj)
objs := $(objs:%.ixx=%.obj)
#test:: ; @echo objs: $(objs)


.SUFFIXES: .c .cpp .ixx

build: info mk_outdirs collect_sources cpp_mod_ifcs $(exe);
	@echo "Main target ready: $(exe)"
info:
	@echo "Build mode: DEBUG=$(DEBUG), CRT=$(CRT), SFML=$(SFML)"

mk_outdirs: $(objdir) $(ifcdir) $(exedir); @#
# Double colons to allow possibly identical dirs:
$(objdir)::; @test -d $@ || mkdir $@
$(ifcdir)::; @test -d $@ || mkdir $@
$(exedir)::; @test -d $@ || mkdir $@

collect_sources:
	@echo Syncing the build cache...
#	This only copies newer files (confirmed with -uia):
	@cp -ufa $(srcdir) $(objdir)
#!! mv can't merge into non-empty dirs with same name, so the 2nd runs failed:
#!! Resorting to keeping the entire src subtree under objdir...
#	mv $(objdir)/$(srcdir)/* -t $(objdir)
#	rmdir $(objdir)/$(srcdir)
#!! This (just like -t) would NOT preserve the src subdirs!
#	cp -u $(SOURCES) $(objdir)/
#	cp -u $(HEADERS) $(objdir)/

cpp_mod_ifcs: $(CPP_MODULE_IFCS)
	@#
	@echo "C++ modules up-to-date."
#	@echo SOURCES: $(SOURCES)
#	@echo objs: $(objs)

$(exe): $(objs)
#	@echo LINK $?
	$(LINK) $(LINKFLAGS) $(objs) $(EXT_LIBS) /out:$@

#-----------------------------------------------------------------------------
# Inference rules...
#-----------------------------------------------------------------------------
.c.obj:
	$(CC) $(CFLAGS) /Fo$(objdir)/ $<
	@mv $(objdir)/`basename $@` -t `dirname $@`

.cpp.obj:
	$(CCC) $(CFLAGS) $(CPPFLAGS) /Fo$(objdir)/ $<
	@mv $(objdir)/`basename $@` -t `dirname $@`

.ixx.ifc::
	$(CCC) $(CFLAGS) $(CPPFLAGS) /Fo$(objdir)/ $<

.ixx.obj::
#	echo OBJ $(CCC) $(CFLAGS) $(CPPFLAGS) /Fo$(objdir)/ $<
	@mv $(objdir)/`basename $@` -t `dirname $@`

#!!?? Why is this never triggered?!
.c:
	@echo .c: cp $< $(objdir)


#-----------------------------------------------------------------------------
# Custom build steps...
#-----------------------------------------------------------------------------
ifdef COMMIT_HASH_INCLUDE_FILE
# BB's pdpmake doesn't support abs. paths for targets! :-/
COMMIT_HASH_INCLUDE_FILE != basename $(COMMIT_HASH_INCLUDE_FILE)
COMMIT_HASH_INCLUDE_FILE := $(outdir)/$(COMMIT_HASH_INCLUDE_FILE)
#debug:; @echo "COMMIT_HASH_INCLUDE_FILE: $(COMMIT_HASH_INCLUDE_FILE)"
$(COMMIT_HASH_INCLUDE_FILE):
#	Using sh's echo explicitly, as this block is about calling an .sh script:
	@echo "Make last commit ID available for \#including..."
	$(SH) "tooling/git/_create_commit_hash_include_file.sh"
	@echo
else
#!! message -ERROR: OON_HASH_INCLUDE_FILE not defined. Env setup was not run?
endif

$(objdir)/src/main.obj: $(COMMIT_HASH_INCLUDE_FILE)

