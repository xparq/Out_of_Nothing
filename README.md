Still just the simple SFML trial project started for a) learning the lib, b) learning
how to architect around frame-based low-level graphics (typical for games, anims.,
visual sims. etc.), c) hopefully distilling a simple framework/toolkit for developing
apps that need this sort of a platform, d) even more vaguely: perhaps actually
developing something interesting out of it -- but now I'm full of fascinating ideas
to explore, as soon as I get through some boring/daunting architectural design +
low-level tech parts first. You can't wait! ;)

![screenshot](asset/image/screenshot_10k_low_friction.png)
------------------------------------------------------------------------------
DEPS.:
	- MSVC (a version supporting c++20, or c++23 if actually using \<format>)
	- SFML 2.6.x (last tried: a4bca205 off the main branch actually)
	- openal32.dll (for audio) - even if linked with the static SFML libs, this
	  will still be loaded (just like the MSVC runtime DLLs -- see below)!

BUILD:

	Set `SFML_LIBROOT=` in `tooling/_setenv.cmd` to match your setup!
	Make sure the MSVC CLI tools (CL, LINK, NMAKE etc.) are on the path!

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


TEST:

	1. Put a `default.ogg` file into the asset dir for background music.
           (In case there wan't one in the package.)

	2. Run `run.cmd` (trivial wrapper to let the .exe find the DLLs).

	3. Check the console for diag. messages (esp. in debug mode).

RELEASE:

	release.cmd will create a .zip file in a local(ly created) tmp dir.

	That package should be self-containing, ready to run anywhere --
	well, provided that the usual MSVC runtime DLLs and opengl32.dll
	are available on the PATH (which should be the case for any sane
	Windows installation).	

------------------------------------------------------------------------------
![screenshot](asset/image/13k-vari-friction-1024.png )
------------------------------------------------------------------------------

NOTES ABOUT THE CODE:

It's still rather horrific here and there (not yet cleaned up all the initial
eager, quick-and-dirty SFML test-driving hackery)!

Unresolved threading-related(?) SFML + OpenGL window/context misery:
[fix-gl-ctx], [fix-setactive-fail] (-> CHANGES.txt)

- SFML-related threading caveats:

	https://stackoverflow.com/a/23921645/1479945
	https://en.sfml-dev.org/forums/index.php?topic=14120.0
	https://en.sfml-dev.org/forums/index.php?topic=24091.0


------------------------------------------------------------------------------
![screenshot](asset/image/screenshot_5000_void_sphere_zoomout_1.png)
------------------------------------------------------------------------------
READ/LEARN/TRY/USE:

Jan Haller ("Nexus" on the SFML forum) is quite the expert, whose stuff is worth checking out:
- Thor lib: https://en.sfml-dev.org/forums/?topic=7329 -> https://bromeon.ch/libraries/thor/, https://github.com/Bromeon/Thor (ARCHIVED!)
  -> https://en.sfml-dev.org/forums/index.php?topic=7329.480
- Aurora C++ helper lib: https://github.com/Bromeon/Aurora (ARCHIVED!)
- SFML book (co-author): https://github.com/SFML/SFML-Game-Development-Book (VERY OLD tho!)

Nero Games:
- their engine: https://nero-games.com/about/, https://github.com/NeroGames/nero-game-engine (using ImGUI)
