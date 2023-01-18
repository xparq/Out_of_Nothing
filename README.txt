Deps..: MSVC (-std:c++20), SFML >= 2.5

??  How the hell is the static-built exe a) so small, b) EVEN SMALLER THAN THE DLL BUILD?!
    Obviously the C++ runtime still comes from a DLL, but still.
    (Yes, the SFML bits are really static: renamed sfml/bin and it still ran fine.)


Threading caveats:

	https://stackoverflow.com/a/23921645/1479945
	https://en.sfml-dev.org/forums/index.php?topic=14120.0

------------------------------------------------------------------------------
TODO:

! Proper colors for bodies finally...

! [fix-gl-ctx] "Failed to activate OpenGL context: The requested resource is in use."...
  -> Possibly unrelated: [fix-random-no-shapes], [fix-setactive-fail]
  ! OK, [fix-setactive-fail] isn't completely unrelated after all, because after fixing
    it (by removing Esc -> window.close() from the event loop) the error changed to:
    	"Failed to activate OpenGL context: Error 3221688541"
    which is just a generic Win32 INVALID OPERATION error... :-/
  ! Also, Esc doesn't trigger the sf::Close event, so why exactly is the Window still 
    closing after all?
  ! Also: if closing the window with its close button, the original error is
    showing yet again.

! [fix-setactive-fail] SFML window.setActive errors on termination.
  -> [fix-gl-ctx] (but doesn't seem to clearly correlate)
  - The one for event_loop happens quite consistently, on exit, even if the gfx thread isn't
    created at all, so no drawing happens whatsoever -- only the HUD ctor loads its font...
    -> ... and, actually, even if it doesn't (do ain'no SFML)!
  - the one for draw is nore sporadic.
  + OK, when I removed the premature window.close() from the event loop, at the
    Esc -> terminate() case, the "resource busy" error of [fix-gl-ctx] changed to:
    	"Failed to activate OpenGL context: Error 3221688541"

? Is there a bounce off the surface, when the globe is moving away while colliding,
  or it's just an optical illusion?! (Shoud be, as there's no bouncing sim! :-o :) )

- Rename "Engine" to sg. less like a generic type but more like a (more specific)
  app "value"... (Game could be fine, but... what if this isn't gonna be a game?)
  -> engine.world is a stupid name, game.world would be perfect, but only for games,
     sim.world, same...
     ? What can be both a game and a sim, and not quite just "app"?...
- Add ref. center pos. to bodies (and explicitly use it as center of mass)!
  + Or at least change it to the middle of them by default!
    -> Transformable::setOrigin?
    (The centering offset is handled by a hardcoded hack in the rendering loop now!)
- A little less grossly wrong gravity pull: scale the vector by distance!
  ! It really doesn't work that way: I mean it may work too well: the distances are
    so vast and the gravity so weak that the moons just don't move at all...
    ? Quite like in the real world, I guess? Should be calculated to check...
- Many-body grav. calc.! (Degrading the algo. from linear to quadratic?... :-/ )
- Fix: zoom makes an offset (panned) view shift sideways unexpectedly.
- Limit (object) v to a) prevent extreme shootoffs, and b) to allow more precise
  collision detection.
  - Also clip minuscule v-s (to 0) allowing to optimize out redundant calc.
- Switch the debug console log on/off, and check (on the HUD) how it affects the fps!
- Measure FPS with partial (vs. the "mandatory" full) screen clear & redraw.
- Test on T400 + Win7!
- Better (de)coupling between SFML and the World!
  The generic World should also be able to use vectors, and time, without
  sf::Vector2 and sf::Clock.
  - Some light adapter ("native generic" <-> SFML) classes could do.
? If performance doesn't mind, move to a less cumbersome design where shapes
  aren't kept persistently all the time, but recreated for each frame anew,
  as needed, and *the way they are needed* (rather than modifying them, e.g.
  by zoom -- a generic Transformable iterator loop might not even have a
  uniform scaling op (e.g. Circles have setRadius())! But setScale() seems OK.).
  BUT: they are likely dynamically allocated, so...


------------------------------------------------------------------------------
DONE:

+ [fix-random-no-shapes] Sometimes (also apparently depending on supposedly unrelated code changes),
  nothing is drawn from the model world, only the HUD! :-o
  -> If the hud instance is not created, the error disappears...
     But if just nothing is called from the HUD, but the ctor, it still happens...

  . Its obscure randomness feels like a threading error, or nullref / access violation.
     . AFAICR it DID NOT start with threading!
     . It started when adding the HUD. Already happened with NO data bindings whatsoever
       in the HUD, just by calling some SFML Text APIs -- and not calling them seemed to have
       "fixed" it for a while...
     . But now, even if HUD drawing & data is disabled, black screens still happen (even more often?)!
     - (One early suspect was clipped overflow text at the right window edge, but it
       soon happened sporadically without that, too.)
  -> Don't seem to clearly correlate neither with [fix-setactive-fail] nor [fix-gl-ctx].
  + Phew! Despite earlier attempts to press 'h' for centering the globe did nothing, the black
    screen was indeed a result of the missing init of OFFSET_*! :-o :-/
