Deps.: MSVC (-std:c++20), SFML >= 2.5

?? How the hell is the static-built exe a) so small, b) EVEN SMALLER THAN THE DLL BUILD?!
   Obviously the C++ runtime still comes from a DLL, but still.
   (Yes, the SFML bits are really static: renamed sfml/bin and it still ran fine.)

TODO:

- Remove SFML dependencies from the World!
  It still has SFML sf::Vector2, at the very least.
  A light vector adapter ("native generic" <-> SFML) class could isolate them.
- Rename "Engine" to sg. less like a generic type but more like a (more specific)
  app "value"... (Game could be fine, but... what if this isn't gonna be a game?)
  -> engine.world is a stupid name, game.world would be perfect, but only for games,
     sim.world, same...
     ? What can be both a game and a sim, and not quite just "app"?...
- A little less grossly wrong gravity pull: scale the vector by distance!
  ! It really doesn't work that way: I mean it may work too well: the distances are
    so vast and the gravity so weak that the moons just don't move at all...
    ? Quite like in the real world, I guess? Should be calculated to check...
- Many-body grav. calc.! (Degrading the algo. from linear to quadratic?... :-/ )
- Add ref. center pos. to bodies!
  + Or at least change it to the middle of them by default!
    (The centering offset is handled by a hardcoded hack in the rendering loop now!)
- Fix: zoom makes an offset (panned) view shift sideways unexpectedly.
- Don't update location (by adding ds) in the middle of the phys. calc., only v!
- Limit (object) v to a) prevent extreme shootoffs, and b) to allow more precise
  collision detection.
- Collisions...
- FPS HUD
	+ debug console log on/off, and check how it affects the fps!
- separate UI thread + check if window drag/resize would still block...
- measure FPS without full screen clear + redraw
- Test on T400 + Win7!
? If performance doesn't mind, move to a less cumbersome design where shapes
  aren't kept persistently all the time, but recreated for each frame anew,
  as needed, and *the way they are needed* (rather than modifying them, e.g.
  by zoom -- a generic Transformable iterator loop might not even have a
  uniform scaling op (e.g. Circles have setRadius())! But setScale() seems OK.).
  BUT: they are likely dynamically allocated, so...
