# Keep the BOM removed, as sh won't find '#' otherwise... ;)
# Also: orig. env var names from Windows are converted to upper case here! :-o

make_exe="nmake /nologo"

make_build_cmd="${make_exe} $*"
make_clean_cmd="${make_exe} clean"

# As a heuristic hack, if `build clean` was explicitly called,
# skip all the magic and just proceed to make...
if [ "$1" == "clean" ]; then
	${make_clean_cmd}
	exit $?
fi


# NMAKE won't let us see its cmdline (from makefiles), so we need to save it here.
save_make_cmd(){
# Also dump the current env., for better change detection in the next run.
# $1 - full make cmdline
# $2 - output file to create/overwrite
	cmd="$1"
	outfile="$2"
	echo _ENV_SNAPSHOT_=\<\<_ENV_END_> "${outfile}"
	env				>> "${outfile}"
	echo _ENV_END_			>> "${outfile}"
	echo				>> "${outfile}"
	echo "$cmd"			>> "${outfile}"
}

new_make_cmd_script="${SZ_OUT_DIR}/make-cmd-PENDING.tmp"
last_make_cmd_script="${SZ_OUT_DIR}/make-cmd-used.tmp"
#echo ${new_make_cmd_script}

save_make_cmd "${make_build_cmd}" "${new_make_cmd_script}"

if [ -f "${last_make_cmd_script}" ] && diff -q "${new_make_cmd_script}" "${last_make_cmd_script}"; then
	# No diff: rotate the cmd files & proceed with increm. build...
	# (That -q may become optional later for retaining the diff for diagnostics!)
	rm "${last_make_cmd_script}"
	mv "${new_make_cmd_script}" "${last_make_cmd_script}"
else
	echo "- Build command/environment changed! Doing full rebuild..."
	${make_clean_cmd}
	# This repetition is pretty stupid & fragile this way, but...:
	save_make_cmd "${make_build_cmd}" "${last_make_cmd_script}"
fi

. ${last_make_cmd_script}
