sz_prjdir="`dirname $0`/../.."
# Well, the SFML-MinGW libs *ALMOST* work with w64devkit, but...: https://github.com/SFML/SFML/issues/1586
# So, SFML would need to be entirely rebuilt for w64devkit specifically,
# just to support these two minor (std::string-related?) incompatibilities. :-/
# This prebuilt lib is not quite enough:
sfml_libroot="C:/SW/devel/lib/sfml/mingw"

# But!... For the time being tho, since -- luckily -- all the link mismatch is
# in the audio stuff, disabling just that allows a successful build! :-o
CC_FLAGS=-DDISABLE_AUDIO $CC_FLAGS

. "${sz_prjdir}/tooling/_setenv.sh"

# Support my "legacy" env. var names:
prjdir=${SZ_PRJDIR:-$sz_prjdir}
src_dir=${SZ_SRC_DIR:-$sz_src_dir}
out_dir=${SZ_OUT_DIR:-$sz_out_dir}
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
	OON_sfml
	Model/World
	renderer_sfml
	hud_sfml
	audio_sfml
"

# Order matters! :-o (See e.g. https://stackoverflow.com/questions/31497702/codeblocks-sfml-audio-library-will-not-work)
LIBS="
	sfml-main sfml-graphics-s.lib sfml-system-s.lib sfml-window-s.lib
	opengl32.lib freetype.lib
	sfml-audio-s.lib vorbisenc.lib vorbisfile.lib vorbis.lib ogg.lib flac.lib openal32.lib
	user32.lib kernel32.lib gdi32.lib winmm.lib advapi32.lib
"

#-----------------------------------------------------------------
INCLUDES="${src_dir}/*.hpp ${src_dir}/*.h"

CC_FLAGS="${CC_FLAGS} -std=c++20"
CC_FLAGS="${CC_FLAGS} -Wall -Wno-unused-value -Wno-switch"
# For GitHub issue #15 (don't rely on manually including cfg.h):
#! CAREFUL with -include: cfg.h exists in w64devkit's stock include set, and would win without the src/! :-o
CC_FLAGS="${CC_FLAGS} -include src/cfg.h"


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
