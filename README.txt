How the fuck is the static-built exe a) so small, b) EVEN SMALLER THAN THE DLL BUILD?!
(Yes, it really is static: renamed sfml/bin and it still ran fine.)

TODO:

- Don't update location (by adding ds) in the middle of the phys. calc., only v!
- Limit (object) v to a) prevent extreme shootoffs, and b) to allow more precise
  collision detection.
- Collisions...
- FPS HUD
	+ debug console log on/off, and check how it affects the fps!
- view panning, home in (center) on objects, o for origin(al pos)
- separate UI thread + check if window drag/resize would still block...
- measure FPS without full screen clear + redraw
- text on T400 + Win7
- remove SFML dependencies from the World!
? If performance doesn't mind, move to a less cumbersome design where shapes
  aren't kept persistently all the time, but recreated for each frame anew,
  as needed, and *the way they are needed* (rather than modifying them, e.g.
  by zoom -- a generic Transformable iterator loop might not even have a
  uniform scaling op (e.g. Circles have setRadius())! But setScale() seems OK.).
  BUT: they are likely dynamically allocated, so...
