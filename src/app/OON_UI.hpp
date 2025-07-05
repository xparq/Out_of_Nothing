/*
 UI abstractions for the rest of the app, to help decoupling from low-level UI
 impl. dependencies

 Well, easier said than done, though... The individual UI widgets often need
 to be accessed directly, like e.g. updating them after a load:

 		gui.set<GravityModeSelector>("Gravity mode", world().gravity_mode);
		gui.set<Slider>("Friction", world().friction);

 - OTOH, *every* widget manipulation should go through some sort of an indirection
   anyway, e.g. to support --headless etc. The current archit. does that at a low
   global level, disabling the GUI but still allowing queries; which the GUI flags:

      "WARNING: Lookup (for "FPS") while the UI is inactive!"

   The current method is more-or-less specific to the UI used; replacing it with
   another would find us with no reusable solution here.

 Short of a "dialog data exchange" (batched-read/update) mechanism (e.g. in the
 GUI itself), we just bite the bullet and include the full GUI here (in its "lying"
 euphemized, wishful-thinking Engine/UI.hpp form, not as the good-old, honest, blunt
 sfw/GUI.hpp...), and continue to support the legacy direct widget access as before.
 (Note: as long as there's direct widget access in each TU *that also uses the
 abstract UI* (which seems to be the case generally), there's no gain e.g. in
 compilation speed by removing the GUI #include from here anyway...)


TODO:
- The ui_... members in OONApp can't just be moved here, unless changed to free
  functions or separated out into a cumbersome UI proxy class.
*/

#pragma once

#include "Engine/UI.hpp"   // Basically sfw/GUI.hpp!
#include "Model/World.hpp" // This one is not so heavy.

namespace UI { class HUD; } //!!...

namespace OON {

	using GravityModeSelector = sfw::OptionsBox<Model::World::GravityMode>;

}
