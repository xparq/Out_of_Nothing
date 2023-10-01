# This is expected to be called from a (temporary) process context, where
# the env. vars won't persist, and won't clash with anything important!

export sz_appname=oon

##!!This $0 is incorrect if we're being sourced by another script in a different dir!!!
export sz_prjdir=${sz_prjdir:-`dirname $0`/..}
sz_prjdir="`realpath ${sz_prjdir}`"

if [ ! -e "${sz_prjdir}/tooling/_setenv.sh" ]; then
	echo "- ERROR: Failed to set the project dir (misplaced ${sz_prjdir}/tooling/_setenv.sh)!"
	exit 1
fi

sfml_libroot=${sfml_libroot:-${sz_prjdir}/extern/sfml/linux}

#echo ${sz_prjdir}
#echo ${sfml_libroot}
#read x

export sz_src_dir=${sz_prjdir}/src
export sz_asset_subdir=asset
export sz_asset_dir=${sz_prjdir}/${sz_asset_subdir}
export sz_out_dir=${sz_prjdir}/out
export sz_run_dir=${sz_prjdir}/run
export sz_tmp_dir=${sz_prjdir}/tmp
export sz_release_dir=${sz_prjdir}/release

export HASH_INCLUDE_FILE=${sz_out_dir}/commit_hash.inc

# CD to prj root for the rest of the process:
cd "$sz_prjdir"

if [ ! -d "${sz_out_dir}" ]; then mkdir "${sz_out_dir}"; fi
if [ ! -d "${sz_out_dir}" ]; then
	echo "- ERROR: Failed to setup project env (${sz_out_dir} was not created)!"
	exit 1
fi

# Note the ; separators, which would fail with GCC on everything, but Windows (MinGW-like?)...
export INCLUDE="${sz_src_dir};extern/sfw/include;${sfml_libroot}/include;${sz_prjdir};${INCLUDE}"
export LIB="${sfml_libroot}/lib;${LIB}"
export PATH="${sz_prjdir}/tooling;${sfml_libroot}/bin;${PATH}"
