A simple SFML trial project a) for learning the lib, b) hopefully distilling
a simple framework/toolkit for developing apps that need this sort of "game-like"
2D platform, c) even more vaguely: perhaps actually developing something interesting
out of it.

![screenshot](https://github.com/lunakid/sfml-test/blob/main/asset/image/screenshot_2500.png)

...Still not clear, what the hell these steerable giant planets should do to each other. :)
Should it be a game (e.g. multiplayer, slingshotting moons at each other, placing balls
into pockets, billiard-like; etc. etc.)? 
Should it be a sim. experiment of e.g. evolutionary trajectory developments, or a 
cellular automata?...

------------------------------------------------------------------------------
DEPS.:
	- MSVC (a version supporting c++20, or c++23 if actually using <format>)
	- SFML 2.6.x (using a4bca205 off the main branch actually)
	- openal32.dll (for audio) - even if linked with the static SFML libs, this
	  will still be loaded (just like the MSVC runtime DLLs -- see below)!

BUILD:

	Set `sfml_libroot=` in `tooling/_setenv.cmd` to match your setup!
	Make sure the MSVC CLI tools (CL, LINK, NMAKE etc.) are on the path!

	Build with statically linked SFML libs:

		build.cmd

	or, for the SFML DLLs:

		build.cmd SFML_DLL=1

	Add DEBUG=1 for debug build.

	Delete out/*.obj (or the whole "out" folder) before switching build modes!

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

	1. Put a default.ogg file into the asset dir for bgnd. music.
	2. Run `run.cmd` (trivial wrapper for finding the DLLs).


RELEASE:

	release.cmd will create a .zip file in a local(ly created) tmp dir.

	That package should be self-containing, ready to run anywhere --
	well, provided that the usual MSVC runtime DLLs and opengl32.dll
	are available on the PATH (which should be the case for any sane
	Windows installation).	

CODE:

Sorry, it's still pretty messy (not yet cleaned up all the initial eager,
quick-and-dirty SFML test-driving hackery)!

Unresolved threading-related(?) SFML + OpenGL window/context misery:
[fix-gl-ctx], [fix-setactive-fail] (-> CHANGES.txt)

- SFML-related threading caveats:

	https://stackoverflow.com/a/23921645/1479945
	https://en.sfml-dev.org/forums/index.php?topic=14120.0
	https://en.sfml-dev.org/forums/index.php?topic=24091.0


------------------------------------------------------------------------------
READ/LEARN/TRY/USE:

Jan Haller ("Nexus" on the SFML forum) is a formidable master, whose stuff is worth checking out:
- Thor lib: https://en.sfml-dev.org/forums/?topic=7329 -> https://bromeon.ch/libraries/thor/, https://github.com/Bromeon/Thor (ARCHIVED!)
  -> https://en.sfml-dev.org/forums/index.php?topic=7329.480
- Aurora C++ helper lib: https://github.com/Bromeon/Aurora (ARCHIVED!)
- SFML book (co-author): https://github.com/SFML/SFML-Game-Development-Book (VERY OLD tho!)

Nero Games:
- their engine: https://nero-games.com/about/, https://github.com/NeroGames/nero-game-engine (using ImGUI)
