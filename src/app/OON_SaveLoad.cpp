#include "OON.hpp"

#include "sfw/GUI.hpp" //!! Used to be in OONApp only, but since scroll_locked() requires it...
                       //!! (And sooner or later it must be usable unrestricted anyway!
#include <iostream>
	using std::cerr;
#include <cassert>

namespace OON {

//----------------------------------------------------------------------------
bool OONApp::load_snapshot(const char* fname) //override
{
	// This should load the model back, but can't rebuild the rendering state:
	if (!SimApp::load_snapshot(fname)) {
		return false;
	}

	//!! NOPE: set_world(world_snapshots[slot]);
	//! Alas, somebody must resync the renderer, too!... :-/
/* A cleaner, but slower way would be:
	//! 1. Halt everything...
	//     DONE, for now, by the event handler!
	//! 2. Purge everything...
	remove_bodies();
	//! 3. Add the bodies back...
	for (auto& bodyptr : world_snapshots[slot]) {
		add_entity(*bodyptr);
	}
*/// Faster, more under-the-hood method:
	//! 1. Halt everything...
	//     DONE, for now, by the event handler!
	//! 2. Purge the renderer only...
// The engine has already written that:
//cerr << "Game state restored from \"" << fname << "\".\n";

	_on_snapshot_loaded();
		//!! (SimApp::load_snapshot could just have a callback directly, instead of
		//!! this current polymorphic setup -- but I expect loading the app state
		//!! to become more than just a pure model loading, and then a virtual
		//!! load_snapshot would become a better match for the task anyway...)

	return true;
}

//----------------------------------------------------------------------------
void OONApp::_on_snapshot_loaded()
//
// Follow-up changes to make sure the app is consistent with what has just been loaded...
//
// NOTE: The loaders are only supposed to validate (consistency-check) the loaded data
//       itself, not anything between that data and the (rest of the) app state!
//
{
	//
	// Sync the UI to reflect the new model state...
	//
	using namespace sfw;

	// Grav. mode:
	if (auto* w = (GravityModeSelector*)gui.recall("Gravity mode"); w)
		w->set(world().gravity_mode);

	//!! The grav. bias widget can only do +/-1000x, so no clean mapping from gravity_strength to that! :-/

	// Drag:
	if (auto* w = (Slider*)gui.recall("Friction"); w)
		w->set(world().friction); //!! The stepping may be inconsistent with the loaded value though! :-o
		                          //!! I think the sfw::Slider will just round it...
	//!!
	//!! ...
	//!!

	oon_main_view().reset(); //!! Technically this doesn't belong to the UI currently.
	                         //!! Kinda considered part of the model, but with some
	                         //!! diegetic UI features (like the grid lines)!
}

} // namespace OON