------------------------------------------------------------------------------
  DEPENDENCIES
------------------------------------------------------------------------------

	- MSVC (a version supporting c++20, or c++23 if actually using \<format>)
	- SFML 2.6.x (last tried: a4bca205 off the main branch actually)
	- openal32.dll (for audio) - even if linked with the static SFML libs, this
	  will still be loaded (just like the MSVC runtime DLLs -- see below)!

------------------------------------------------------------------------------
  BUILD
------------------------------------------------------------------------------

	Set `SFML_LIBROOT=` in `tooling/_setenv.cmd` to match your setup!
	(And of course anything else, if needed.)

	It really only has been built on Windows (10) yet.

	Make sure the MSVC CLI tools (CL, LINK, NMAKE etc.) are on the PATH.
	Oh, and an instance of [BusyBox.exe](https://frippery.org/files/busybox/busybox.exe) (but that goes without saying, as
	everybody has one already, right? ;) )

	Build with static-linked SFML libs:

		build.cmd

	or, with the SFML DLLs:

		build.cmd SFML_DLL=1

	Add DEBUG=1 for debug build.

	Also see (and change) the conditional compilation options in cfg.h.

	(If you forget to run `build clean` before changing the build mode, no
	worries: the build script detects it for you, and triggers a full rebuild
	to avoid mismatched binaries.)

	Note: Even with a static build, openal32.dll (shipped with SFML) will still
	      be linked dynamically!

	(If you use the binary release of SFML, and would like a fully static link
	that also includes the MSVC runtime, trying to compile with -MT here would
	be futile... SFML is built with -MD (for loading the MSVC runtime from
	DLLs), so it would require a complete SFML rebuild from sources with -MT.
	Oh, and openal32.dll still won't be compiled in, of course.)

??	For quite a while, the static-linked exe was pretty small, <300K. Then
	I haven't checked for a while, and when I added sound, that made it
	~2MB immediately. Then, even if I removed all the SFML sound code, the
	exe was still >800K (built from scratch)! Why?!

??	Why the hell was the static-built exe EVEN SMALLER THAN THE DLL BUILD?!
	(Only when built with no audio, and in the early 300K days; always fully
	rebuilt with a single CL command in the old batch files.)

??	Why the hell was there a significant difference between a static exe size
	when built with a single CL command, and when built from separate .obj
	modules?! (Using the same CL options.)

- RELEASE PACKAGING:

	No GitHub action for it yet. For manual packaging:

		release.cmd

	will create a .zip file in a local(ly created) tmp dir.

	That package should be self-containing, ready to run anywhere (well,
	provided that the usual MSVC runtime DLLs and opengl32.dll are all
	available on the PATH at the target machine, which should be the
	case for any sane Windows installation at least).


------------------------------------------------------------------------------
  TEST
------------------------------------------------------------------------------

	1. Put a `default.ogg` file into the asset dir for background music.
           (In case there wan't one in the package.)

	2. Run `run.cmd` (trivial wrapper to let the .exe find the DLLs).

	3. Check the console for diag. messages (esp. in debug mode).

------------------------------------------------------------------------------
  IMPLEMENTATION
------------------------------------------------------------------------------

- SFML-related threading caveats:

	https://stackoverflow.com/a/23921645/1479945
	https://en.sfml-dev.org/forums/index.php?topic=14120.0
	https://en.sfml-dev.org/forums/index.php?topic=24091.0

- CODE QUALITY:

	Much of it is still rather horrific prototype code, not yet cleaned up from
	all the initial eager, quick-and-dirty SFML test-driving hackery.

	Also, an ongoing unresolved threading-related(?) SFML + OpenGL window/context
	misery: [fix-gl-ctx], [fix-setactive-fail] (-> CHANGES.txt)
	Doesnt' seem to affect anything noticable though... Which wasn't always the
	case (e.g. full-screen switching didn't work), but it just kinda went away. :-o
	(Hopefully as a side-effect of two earlier C++-level memory-corrupting bugs.)


------------------------------------------------------------------------------
  READ/LEARN/TRY/USE
------------------------------------------------------------------------------

Jan Haller ("Nexus" on the SFML forum) is quite the expert, whose stuff is worth checking out:
- Thor lib: https://en.sfml-dev.org/forums/?topic=7329 -> https://bromeon.ch/libraries/thor/, https://github.com/Bromeon/Thor (ARCHIVED!)
  -> https://en.sfml-dev.org/forums/index.php?topic=7329.480
- Aurora C++ helper lib: https://github.com/Bromeon/Aurora (ARCHIVED!)
- SFML book (co-author): https://github.com/SFML/SFML-Game-Development-Book (VERY OLD tho!)

Nero Games:
- their engine: https://nero-games.com/about/, https://github.com/NeroGames/nero-game-engine (using ImGUI)
