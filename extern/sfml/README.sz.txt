This ("pseudo-pseudocode") CMD snippet would setup SFML for the OON build:
{
	:: This just to set SZ_SFML_LIBROOT the same as for the build:
	call tooling\_setenv.sh

	:: Download stock libs:
	tooling\sfml_dl+unzip.cmd

	:: Download custom full-static addon libs:
	tooling\busybox sh tooling/sfml-setup-MSVCRT-static-addon
}

(It's done almost the same by the GHA build, just setting SZ_SFML_LIBROOT
differently.)
