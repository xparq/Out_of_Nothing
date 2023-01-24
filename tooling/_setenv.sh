# This is expected to be called from a (temporary) process context, where
# the env. vars won't persist, and won't clash with anything important!

sz_appname=sfml-test

sz_prjdir=${sz_prjdir:-$(dirname $0)/..}
if [ ! -f "${sz_prjdir}/tooling/_setenv.sh" ]; then
	echo "- ERROR: Failed to set the project dir (misplaced ${sz_prjdir}/tooling/_setenv.sh)!"
	exit 1
fi

sfml_libroot=${sfml_libroot:-${sz_prjdir}/../../sfml/current}

#echo ${sz_prjdir}
#echo ${sfml_libroot}
#read x

INCLUDE="${sfml_libroot}/include;${INCLUDE}"
LIB="${sfml_libroot}/lib;${LIB}"
PATH="${sz_prjdir}/tooling;${sfml_libroot}/bin;${PATH}"

sz_src_dir=${sz_prjdir}/src
sz_asset_subdir=asset
sz_asset_dir=${sz_prjdir}/${sz_asset_subdir}
sz_out_dir=${sz_prjdir}/out
sz_tmp_dir=${sz_prjdir}/tmp
sz_release_dir=${sz_prjdir}/release

if [ ! -d "${sz_out_dir}" ]; then mkdir "${sz_out_dir}"; fi
if [ ! -d "${sz_out_dir}" ]; then
	echo "- ERROR: Failed to setup project env (${sz_out_dir} was not created)!"
	exit 1
fi

HASH_INCLUDE_FILE=${sz_out_dir}/commit_hash.inc
