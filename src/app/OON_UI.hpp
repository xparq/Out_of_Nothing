/*
 UI abstractions for the rest of the app, to help decoupling from low-level or
 "inconveniently specific" UI impl. dependencies

 (This is just the internal interface "language", to be included in headers like
 OON.hpp. See OON_UI-impl.hpp for defs used by TUs *implementing* UI features!)

 Well, easier said than done, though... The individual UI widgets often need
 to be accessed directly, like e.g. updating them after a load:

		gui.set<GravityModeSelector>("Gravity mode", world().props.gravity_mode);
		gui.set<Slider>("Friction", world().props.friction);

 - OTOH, *every* widget manipulation should go through some sort of an indirection
   anyway, e.g. to support --headless etc. The current archit. does that at a low
   global level, disabling the GUI but still allowing queries; which the currently
   used GUI doesn't fully support, and flags as suspicious:

      "WARNING: Lookup (for "FPS") while the UI is inactive!"

   The current method is more-or-less specific to the UI used (mycoGUI); replacing
   it would leave us with an incompatible solution here.

 Short of a "dialog data exchange" (batched-read/update) mechanism (e.g. in the
 GUI itself), we just bite the bullet and include the full GUI here (in its "lying"
 euphemized, wishful-thinking Szim/UI.hpp form, not as the good-old, honest, blunt
 myco/GUI.hpp...), and continue to support the legacy direct widget access as before.
 (Note: as long as there's direct widget access in each TU *that also uses the
 "abstract" UI* (which seems to be the case generally), there's no gain e.g. in
 compilation speed by removing the GUI #include from here anyway...)


TODO:
- The ui_... members in OONApp can't just be moved here, unless changed to free
  functions or separated out into a cumbersome UI proxy class.
*/

#pragma once

#include "Szim/UI.hpp"  // Basically myco/GUI-main.hpp... Unfortunately, can't just
                        // fw-declare it: myco::Options<...> below needs it!... :-/

#include "myco/widget/Options.hpp" //!! Should come via a "Szm/UI/Options.hpp" proxy header!
#include "Szim/UI/Banner.hpp"
// This one doesn't seem to need the full def for OON.h...
namespace Szim::UI { class HUDStream; }

#include "app/Model/World.hpp" // This one is not so heavy.
namespace OON
{
	using GravityModeSelector   = myco::Options<Model::GravityMode>;   //!! -> Szim::UI...
	using CollisionModeSelector = myco::Options<Model::CollisionMode>; //!! -> Szim::UI...

	using PauseBanner = Szim::UI::Banner;
}
