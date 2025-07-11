NOTE:

- This UI is for Engine use, i.e. diagnostics, app/model editing, in-game
  admin etc. only!

  It does NOT try to be the universal, one-size-fits-all UI solution for every
  possible app (game)...

  But it does strive for presenting a decent, usable out-of-the-box, initial UI
  solution for apps, just to start out with something, easing early development!
  (Similar strategy to e.g. logging or the other "externally usable internal"
  services available in the Engine.)


*
Integrate properly: make it "backended" like the rest.

*
The Engine should strive to minimize its daunting dependencies on any full-fledged
UI lib/framework (and then specific platforms, backend etc.), so ideally it should
think in terms of abstract UI integration points, to make any specific UI impls.
easily replaceable.

*
Prepare for multiple UI options (even multiple categories: GUI, console,
multimodal etc.) being offered later.

*
Keep `--headless` in mind...

