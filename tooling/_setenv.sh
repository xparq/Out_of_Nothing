# This is expected to be called from a (temporary) process context, where
# the env. vars won't persist, and won't clash with anything important!

export SZ_APP_NAME=oon

##!!This $0 is incorrect if we're being sourced by another script in a different dir!!!
export SZ_PRJDIR=${SZ_PRJDIR:-`dirname $0`/..}
SZ_PRJDIR="`realpath ${SZ_PRJDIR}`"

if [ ! -e "${SZ_PRJDIR}/tooling/_setenv.sh" ]; then
	echo "- ERROR: Failed to set the project dir (misplaced ${SZ_PRJDIR}/tooling/_setenv.sh)!"
	exit 1
fi

export SZ_SFML_ROOT=${SZ_SFML_ROOT:-${SZ_PRJDIR}/extern/sfml/msvc}

#echo SZ_PRJDIR = ${SZ_PRJDIR}
#echo SZ_SFML_ROOT = ${SZ_SFML_ROOT}
#echo Press Enter to proceed...
#read x

export SZ_SRC_SUBDIR=src
export SZ_OUT_SUBDIR=build.out
export SZ_IFC_SUBDIR=ifc
export SZ_TEST_SUBDIR=test
export SZ_RUN_SUBDIR=${SZ_TEST_SUBDIR}
export SZ_ASSET_SUBDIR=asset
export SZ_RELEASE_SUBDIR=release

export SZ_SRC_DIR=${SZ_PRJDIR}/${SZ_SRC_SUBDIR}
export SZ_OUT_DIR=${SZ_PRJDIR}/${SZ_OUT_SUBDIR}
export SZ_TEST_DIR=${SZ_PRJDIR}/${SZ_TEST_SUBDIR}
export SZ_RUN_DIR=${SZ_TEST_DIR}
export SZ_ASSET_DIR=${SZ_PRJDIR}/${SZ_ASSET_SUBDIR}
export SZ_TMP_DIR=${SZ_PRJDIR}/tmp
export SZ_RELEASE_DIR=${SZ_TMP_DIR}/${SZ_RELEASE_SUBDIR}

export COMMIT_HASH_INCLUDE_FILE=${SZ_OUT_DIR}/commit_hash.inc

# CD to prj root for the rest of the process/action:
cd "$SZ_PRJDIR"
res=$?
if [ $res -ne 0 ]; then
	echo "- ERROR: Failed to enter project dir ($SZ_PRJDIR)?!"
	exit $res
fi


if [ ! -d "${SZ_OUT_DIR}" ]; then mkdir "${SZ_OUT_DIR}"; fi
if [ ! -d "${SZ_OUT_DIR}" ]; then
	echo "- ERROR: Failed to setup project env (${SZ_OUT_DIR} was not created)!"
	exit 1
fi

# Note the ; separators, which would fail with GCC on everything, but Windows (MinGW-like?)...
export INCLUDE="${SZ_SRC_DIR};extern;extern/sfw/include;${SZ_SFML_ROOT}/include;${SZ_PRJDIR};${INCLUDE}"
export LIB="${SZ_SFML_ROOT}/lib;${LIB}"
export PATH="${SZ_PRJDIR}/tooling;${SZ_SFML_ROOT}/bin;${PATH}"
