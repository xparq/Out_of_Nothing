...Still not clear, what the hell these steerable giant planets should do to each other. :)
Should it be a game (e.g. multiplayer, slingshotting moons at each other, placing balls
into pockets, billiard-like; etc. etc.)? 
Should it be a sim. experiment of e.g. evolutionary trajectory developments, or a 
cellular automata?...

------------------------------------------------------------------------------
DEVELOPMENT:

Deps.:
	MSVC (-std:c++latest -- ++20 is fine, unless actually using <format>)
	SFML 2.6.x (using a4bca205 off the main branch actually)

Build:
	??  Why the hell is the static-built exe a) so small, b) EVEN SMALLER THAN THE DLL BUILD?!
	    (Obviously the C++ runtime still comes from a DLL, but still.
	    Yes, confirmed the SFML bits are really statically linked, by renaming sfml/bin
	    and it still seeing it run fine.)


SFML-related threading caveats:

	https://stackoverflow.com/a/23921645/1479945
	https://en.sfml-dev.org/forums/index.php?topic=14120.0
	https://en.sfml-dev.org/forums/index.php?topic=24091.0

	Unresolved threading-related(?) SFML + OpenGL window/context misery:
	[fix-gl-ctx], [fix-setactive-fail] (-> CHANGES.txt)

------------------------------------------------------------------------------
READ/LEARN/TRY/USE:

	Jan Haller ("Nexus" on the SFML forum) is a seasoned pro, whose stuff is worth checking out:
		Thor lib: https://en.sfml-dev.org/forums/?topic=7329 -> https://bromeon.ch/libraries/thor/, https://github.com/Bromeon/Thor (ARCHIVED!)
			-> https://en.sfml-dev.org/forums/index.php?topic=7329.480
		Aurora C++ helper lib: https://github.com/Bromeon/Aurora (ARCHIVED!)
		SFML book (co-author): https://github.com/SFML/SFML-Game-Development-Book (VERY OLD tho!)

	https://nero-games.com/about/, https://github.com/NeroGames/nero-game-engine (using ImGUI)
