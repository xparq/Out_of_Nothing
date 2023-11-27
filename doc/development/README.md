[WARNING! There has been some updates to this near the end of 2023, but bits
and pieces of it may still be OBSOLETE!]

------------------------------------------------------------------------------
  RUN
------------------------------------------------------------------------------

  Dependencies:

	Releases are supposed to be self-containing, and shouldn't need
	installing, except "of course" the MSVC runtime DLLs, which have
	likely been installed already, but if not, follow the README of
	the specific release.

	For building from sources, read on.

------------------------------------------------------------------------------
  BUILD
------------------------------------------------------------------------------

  Dependencies:

	- MSVC (a version supporting c++20, or c++23 if actually using \<format>)
	- SFML 3 (tested e.g.: vc17-64 3f6403e279d3f0965938250f263cdd9fdfef5f4a)
	- openal32.dll (for audio) - even if linked with the static SFML libs, this
	  will still be loaded dynamically (like the MSVC runtime DLLs, see below)!

	Set `SFML_LIBROOT=` in `tooling/_setenv.cmd` to match your setup!
	(And of course anything else, if needed.)

	It really only has been built on Windows (10) yet.

	Make sure the MSVC CLI tools (CL, LINK, NMAKE etc.) are on the PATH.
	Oh, and an instance of [BusyBox.exe](https://frippery.org/files/busybox/busybox.exe)
	(but that goes without saying, as everybody has one already, right? ;) )

	Build with static-linked SFML libs:

		bbuild.cmd

	or, with the SFML DLLs:

		bbuild.cmd SFML=dll

	Add DEBUG=1 for debug builds.

	Also see the supported conditional compilation options in

		Engine/_build_cfg.h

	and change them preferably via command line options to the build script.


	NOTES:

	    - The DEBUG and SFML static/dll build variants are kept separate
	      in parallel dir trees, so they can be built/tested independently.

	    - The NMAKE-based build procedure (nbuild) does not yet have
	      automatic header dependency tracking! (The BB-based bbuild does.)

	    - Even with a static build, openal32.dll (shipped with SFML) will
	      still be linked dynamically!

!!	If you use the binary release of SFML, and would like full static link
	that also includes the MSVC runtime, trying to compile with -MT would
	be futile... SFML is built with -MD (for loading the MSVC runtime from
	DLLs), so it would require a complete SFML rebuild with -MT.
	(And, remember, openal32.dll still can't be linked statically.)

??	For quite a while, the static-linked exe was pretty small, <300K. Then
	I haven't checked for a while, and when I added sound, that made it
	~2MB immediately. Then, even if I removed all the SFML sound code, the
	exe was still >800K! Why?!

??	Why the hell was the static-built exe EVEN SMALLER THAN THE DLL BUILD?!
	(Only when built with no audio, and in the early 300K days; always fully
	rebuilt with a single CL command in the old batch files.)

??	Why the hell was there a significant difference between a static exe size
	when built with a single CL command, and when built from separate .obj
	modules?! (Using the same CL options, with no LTO in either cases.)


------------------------------------------------------------------------------
  [OBSOLETE!] RELEASE PACKAGING
------------------------------------------------------------------------------

	No GitHub action for it yet. For manual packaging:

		release.cmd

	will create a .zip file in a local(ly created) tmp dir.

	That package should be self-containing, ready to run anywhere -- well,
	provided that the usual MSVC runtime DLLs are installed on the target
	machine.


------------------------------------------------------------------------------
  TEST
------------------------------------------------------------------------------

	1. Put a `default.ogg` file into the asset dir for background music.
           (In case there wasn't one in the package.)

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

	There's still some embarrassing prototype code not yet cleaned up
	from the initial eager, quick-and-dirty "just an SFML test hackery"
	days.

	Also, an ongoing unresolved (threading-related?) SFML + OpenGL
	window/context misery: [fix-gl-ctx], [fix-setactive-fail] (-> CHANGES.txt)
	Doesnt' seem to affect anything important though... Albeit full-screen
	switching does work very haphazardly (-- and the problem even just
	went away for a while, and then returned in later versions... :-o ).


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
