# Keep the BOM removed, as sh won't find '#' otherwise... ;)
# Also: orig. env var names from Windows are converted to upper case here! :-o

make_cmd="nmake /nologo $*"
#echo ${make_cmd}

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

make_cmd_script="${SZ_OUT_DIR}/make-cmd.sh"
#echo ${make_cmd_script}

save_make_cmd "${make_cmd}" "${make_cmd_script}"

. "${make_cmd_script}"
