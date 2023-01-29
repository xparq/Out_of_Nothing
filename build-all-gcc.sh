sz_prjdir="`dirname $0`"
# Well, the SFML-MinGW libs *ALMOST* work with w64devkit, but...: https://github.com/SFML/SFML/issues/1586
# So, something, but not quite... this would be needed to actually link successfully:
sfml_libroot="C:/SW/devel/lib/sfml/mingw"
	# Actually: SFML would need to be entirely rebuilt for w64devkit specifically,
	# just to support this one minor (std::string-related) incompatibilities. :-/

. "${sz_prjdir}/tooling/_setenv.sh"

# Support my "legacy" env. var names:
prjdir=$SZ_PRJDIR
src_dir=$SZ_SRC_DIR
out_dir=$SZ_OUT_DIR
#Why the FUCK did this only work in lowercase?!
appname=$sz_appname

src_dir=${src_dir:-./src}
out_dir=${out_dir:-./tmp}

#if "${src_dir}" == "" || "${out_dir}" == ""
#error - Build env. not initialized (correctly)!
#endif

EXE=${out_dir}/${appname}.exe

MODULES="
	main
	world_sfml
	renderer_sfml
	engine_sfml
	hud_sfml
	audio_sfml
"

# Order matters! :-o (See e.g. https://stackoverflow.com/questions/31497702/codeblocks-sfml-audio-library-will-not-work)
LIBS="
	sfml-main sfml-graphics-s.lib sfml-system-s.lib sfml-window-s.lib
	sfml-audio-s.lib vorbisenc.lib vorbisfile.lib vorbis.lib ogg.lib flac.lib openal32.lib
	opengl32.lib freetype.lib
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
"

#-----------------------------------------------------------------
INCLUDES="${src_dir}/*.hpp ${src_dir}/*.h"

#CC_FLAGS="${CC_FLAGS} -Wall --std=c++20"
CC_FLAGS="${CC_FLAGS} --std=c++20"
# For GitHub issue #15 (don't rely on manually including cfg.h):
CC_FLAGS="${CC_FLAGS} -include cfg.h"
CC_CMD="cl -nologo -c ${CC_FLAGS} -Fo${out_dir}/ -Fd${out_dir}/"
BB=busybox
ECHO=${BB} echo



#-----------------------------------------------------------------
for m in ${MODULES}; do
#If converting from MSVC:
#	OBJS="$OBJS ${m%.o}.cpp"
#	OBJS="$OBJS ${out_dir}/${m}.o"
	srcs="$srcs ${src_dir}/${m}.cpp"
done
#echo $srcs

for l in ${LIBS}; do
	libs="$libs -l${l%.lib}"
#	libs="$libs -l${l}"
done
#echo $libs

export CPLUS_INCLUDE_PATH="$INCLUDE;$CPLUS_INCLUDE_PATH"
export    LIBRARY_PATH="$LIB;$LIBRARY_PATH"

cmd="g++ ${CC_FLAGS} -DSFML_STATIC -L${LIB} $srcs $libs -o${out_dir}/${appname}-gcc.exe"
echo $cmd
$cmd
	