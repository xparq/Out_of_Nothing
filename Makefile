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
#!! Can't be empty, for error-prone $(...DIR)/... concats (yet)!
SZ_RUN_SUBDIR ?= .

exe = $(exedir)/$(SZ_APP_NAME)$(cflags_crt_linkmode_with_debug)$(buildmode_sfml_linkmode_tag).exe

outdir = $(SZ_OUT_SUBDIR)
#!!objdir = $(vroot)/obj
#!!Alas...:
objdir = $(vroot)/$(SZ_SRC_SUBDIR)
#!!ifcdir = $(vroot)/ifc
#!!Alas...:
ifcdir = $(vroot)/$(SZ_SRC_SUBDIR)
exedir = $(SZ_RUN_SUBDIR)

#!! Kludge until I find out the correct way to support C++ modules...
#!!Use != ls -b or something *.ifc!
#!!CPP_MODULE_IFCS = $(ifcdir)/*.ifc
CPP_MODULE_IFCS = $(ifcdir)/Storage.ifc
#!! Also, any module change will trigger full rebuild, coz no sane way
#!! to auto-track actual deps yet (even less than headers)! :-/
#!! (E.g. https://stackoverflow.com/questions/69190433/auto-generate-dependencies-for-modules,
#!! https://github.com/premake/premake-core/issues/1735)
#!! So, well, sigh... (or add a manaully updated list...):
objs_using_modules = $(objs)

EXT_LIBS =\
	$(SZ_SFML_LIBROOT)/lib/$(sfml_and_deps_libs) \
	extern/sfw/lib/msvc/sfw$(cflags_crt_linkmode_with_debug)$(buildmode_sfml_linkmode_tag).lib \
	extern/zstd/msvc/zstd$(cflags_crt_linkmode).lib \
	user32.lib\

# Adapt the various flavors of the SFML libs (as per the debug/link modes)...
SFML_linkmode=$(SFML)

_sfml_libs__static = sfml-graphics-s.lib sfml-window-s.lib sfml-system-s.lib sfml-audio-s.lib
_sfml_libs__dll    = sfml-graphics.lib   sfml-window.lib   sfml-system.lib   sfml-audio.lib
sfml_libs = $(_sfml_libs__$(SFML_linkmode))

_sfml_and_deps_libs__static = $(sfml_libs) opengl32.lib freetype.lib\
	ogg.lib vorbis.lib vorbisenc.lib vorbisfile.lib flac.lib openal32.lib \
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
_sfml_and_deps_libs__dll = $(sfml_libs) opengl32.lib
sfml_and_deps_libs = $(_sfml_and_deps_libs__$(SFML_linkmode))
#debug:; @echo sfml_and_deps_libs: $(sfml_and_deps_libs)

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
_cflags_debug__0 = -DNDEBUG /O2
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
# Also: CRT=dll is always the case with the pre-built SFML libs, so no use changing it...
##!!
##!! Instead of the horror of having 3 sets of these macros (a generic tag,
##!! a dir tag, a file tag -- where each must separately dispatch on their
##!! parameter!), use a uniform format suitable for each purpose (as a suffix):
##!!
_buildmode_debug_tag__0 =
_buildmode_debug_tag__1 = .d
buildmode_debug_tag     = $(_buildmode_debug_tag__$(DEBUG))
_buildmode_crtdll_tag__static =
_buildmode_crtdll_tag__dll    = .crtdll
buildmode_crtdll_tag          = $(_buildmode_crtdll_tag__$(CRT))
_buildmode_sfml_linkmode_tag__static =
_buildmode_sfml_linkmode_tag__dll    = .SFMLdll
buildmode_sfml_linkmode_tag          = $(_buildmode_sfml_linkmode_tag__$(SFML_linkmode))

# All combined:
buildmode_dir_tag  = $(buildmode_crtdll_tag)$(buildmode_sfml_linkmode_tag)$(buildmode_debug_tag)
buildmode_file_tag = $(buildmode_crtdll_tag)$(buildmode_sfml_linkmode_tag)$(buildmode_debug_tag)
#debug:; @echo buildmode_dir_tag: $(buildmode_dir_tag)
#debug:; @echo buildmode_file_tag: $(buildmode_file_tag)

_sfml_lib_debug_suffix__0 =
_sfml_lib_debug_suffix__1 = -d
sfml_lib_debug_suffix     = $(_sfml_lib_debug_suffix__$(DEBUG))
# Mind the mandatory := here, to avoid macro recursion:
sfml_libs := $(sfml_libs:%.lib=%$(sfml_lib_debug_suffix).lib)
#debug:; @echo sfml_libs: $(sfml_libs)

#--- CUSTOMIZATIONS ----------------------------------------------------------
CL   := cl /nologo
CC   := $(CL) -c
CXX  := $(CL) -c /EHsc
LINK := link.exe /nologo
SH   := busybox sh


# Not using += because we're prepending, to support overriding (e.g. from the CMDLINE)!
#! Kludge: avoid recursive assignment, while still allowing deferred expansion!
#_CFLAGS := $(CFLAGS)
CFLAGS   = $(cflags_crt_linkmode_with_debug) $(cflags_debug)
#!! Incomplatible with /RTCs if DEBUG, so moved back to the debug dispatching:
#!!CFLAGS  += /O2
# Add one also for including the commit hash file:
CFLAGS  += /I$(objdir) /I$(outdir)
CFLAGS  += $(_CFLAGS)
#_CPPFLAGS := $(CPPFLAGS)
CPPFLAGS   = $(CFLAGS) /std:c++latest /ifcOutput $(ifcdir)/ /ifcSearchDir $(ifcdir)/
CPPFLAGS  += $(cflags_sfml_linkmode)
CPPFLAGS  += $(_CPPFLAGS)
#_LINKFLAGS := $(LINKFLAGS)
LINKFLAGS   = $(linkflags_debug)
LINKFLAGS  += $(linkflags_debug) $(_LINKFLAGS)


#-----------------------------------------------------------------------------
# BUILD "ENGINE"
#-----------------------------------------------------------------------------

# Preparations...

# This is the build-mode-specific (and accordingly tagged) "VPATH" root:
vroot = $(SZ_OUT_SUBDIR)/v$(buildmode_dir_tag)

SOURCES != busybox sh -c "find $(SZ_SRC_SUBDIR) -name '*.c' -o -name '*.cpp' -o -name '*.ixx'"
#!!?? Couldn't get the BB `find` syntax right without the `sh -c` kludge! :-/
SOURCES := $(SOURCES:$(SZ_SRC_SUBDIR)/%=%)
#test:: ; @echo SOURCES: $(SOURCES)

# No need for such pattern shenanigans, as the current dir (./) has
# been appended by `find`
#!!_src_prep := FPFX_PLACEHOLDER_$(SOURCES:%.c=FPFX_PLACEHOLDER_%.c)
objs := $(SOURCES:%=$(objdir)/%)
#dump:; @echo objs: $(objs)
objs := $(objs:%.c=%.obj)
objs := $(objs:%.cpp=%.obj)
objs := $(objs:%.ixx=%.obj)
#dump:; @echo objs: $(objs)
#!! No idea how this would translate to 2-file module sources though:
#!! CPP_MODULE_IFCS already has $(ifcdir), which is currently identical to $(objdir), so...
objs_of_modules = $(CPP_MODULE_IFCS:%.ifc=%.obj)
#dump:; @echo objs_of_modules: $(objs_of_modules)

#-----------------------------------------------------------------------------
# List the phonies so `make -t` won't create empty files for them:
.PHONY: build collect_sources cpp_modules info mk_outdirs

build: info mk_outdirs collect_sources cpp_modules $(exe);
	@echo "Main target ready: $(exe)"
info:
	@echo "Build mode: DEBUG=$(DEBUG), CRT=$(CRT), SFML=$(SFML)"

_mkdir = test -d $@ || mkdir $@
mk_outdirs: $(outdir) $(vroot) $(objdir) $(ifcdir) $(exedir); @#
# Double colons in order to not fail on possibly identical dirs:
$(outdir)::; @$(_mkdir)
$(vroot)::;  @$(_mkdir)
$(objdir)::; @$(_mkdir)
$(ifcdir)::; @$(_mkdir)
$(exedir)::; @$(_mkdir)

collect_sources:
	@echo Syncing the build cache...
#	This (cp -u) only copies newer files (confirmed with -uia):
	@cp -ufa $(SZ_SRC_SUBDIR) $(vroot)
#	As cp can only add and overwrite but not delete existing files,
#	the cached tree must be deleted from time to time to remove cruft!
	@touch $(vroot)/.cached_src_age
	@_cached_src_age=$$((`cat $(vroot)/.cached_src_age`)) &&\
		if [ "$$_cached_src_age" != "10" ]; then\
			echo "$$((_cached_src_age + 1))"> $(vroot)/.cached_src_age;\
		else\
			echo "[!!NOT IMPLEMENTED YET!!] Src cache too old, deleting...";\
			echo 0> $(vroot)/.cached_src_age;\
		fi
#!! mv can't merge into non-empty dirs with same name, so the 2nd runs failed:
#!! Resorting to keeping the entire src subtree under objdir...
#	mv $(objdir)/$(srcdir)/* -t $(objdir)
#	rmdir $(objdir)/$(srcdir)
#!! This (just like -t) would NOT preserve the src subdirs!
#	cp -u $(SOURCES) $(objdir)/
#	cp -u $(HEADERS) $(objdir)/

cpp_modules:: $(CPP_MODULE_IFCS) $(objs_of_modules)
	@echo "C++ modules up-to-date."

# (Sigh... See the issue at the def. of objs_using_modules!)
#!!?? Is adding $(objs_of_modules) actually necessary?
#!!??$(objs_using_modules) $(objs_of_modules): $(CPP_MODULE_IFCS)
$(objs_using_modules): $(CPP_MODULE_IFCS)

# Strip the ugly paths from each object
#!! Alas, can't be done without my path-inference hack to pdpmake:
#!!$(exe): $(objs:$(objdir)/%=%)
#!!	@echo LINK $?
#!!	$(LINK) $(LINKFLAGS) /libpath:$(objdir) $(objs) $(EXT_LIBS) /out:$@
$(exe): $(objs)
	$(LINK) $(LINKFLAGS) $(objs) $(EXT_LIBS) /out:$@

#-----------------------------------------------------------------------------
# Inference rules...
#-----------------------------------------------------------------------------
.SUFFIXES: .c .cpp .ixx

.c.obj:
	$(CC) $(CFLAGS) /Fo$(objdir)/ $<
	@mv $(objdir)/`basename $@` -t `dirname $@`

.cpp.obj:
	$(CXX) $(CPPFLAGS) /Fo$(objdir)/ $<
	@mv $(objdir)/`basename $@` -t `dirname $@`

.ixx.ifc:
	$(CXX) $(CPPFLAGS) /Fo$(objdir)/ $<

.ixx.obj:
	$(CXX) $(CPPFLAGS) /Fo$(objdir)/ $<
	@mv $(objdir)/`basename $@` -t `dirname $@`

#-----------------------------------------------------------------------------
# Custom rules...
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

$(outdir)/src/main.obj: $(COMMIT_HASH_INCLUDE_FILE)

