#include "OON.hpp"
#include "OON_UI.hpp" // GravityModeSelector

//#include "Szim/diag/Error.hpp"
//#include "Szim/diag/Log.hpp"
#include <cassert>

namespace OON {

//----------------------------------------------------------------------------
#if 0 //!! OLD:
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

	on_snapshot_loaded();
		//!! (SimApp::load_snapshot could just have a callback directly, instead of
		//!! this current polymorphic setup -- but I expect loading the app state
		//!! to become more than just a pure model loading, and then a virtual
		//!! load_snapshot would become a better match for the task anyway...)

	return true;
}
#endif //!! OLD

//----------------------------------------------------------------------------
void OONApp::on_snapshot_loaded() //override
//
// Follow-up changes to make sure the app is consistent with what has just been loaded
//
// NOTE: The loaders are only supposed to validate (consistency-check) the loaded
//       model data itself, not anything between that data and the (rest of the) app state!
//!!     However: an app-level load() override is still a perfectly legit place to do any
//!!     post-model-load fixup chores there, rather than via this callback!
//!!     But there's just no virtual app::save/load currently. (Which may change!)
//
{
	if (!cfg.headless) { //!! This shouldn't be handled in this scattered manner... :-/
	                     //!! (See also at the HCI/Window adapter, or the disabling of the main event loop...)
		//
		// Sync the UI to reflect the new model state...
		//

		//!! Move these to OON_UI, and only call those here, instead of the direct UI manip.!
		using namespace myco;

		// Grav. mode:
		//!! The grav. bias widget can only do +/-1000x, so no clean mapping from gravity_strength to that! :-/
		gui.set<GravityModeSelector>("Gravity mode", world().props.gravity_mode);

		// Drag:
		gui.set<Slider>("Friction", world().props.friction);
			//!! The stepping may be inconsistent with the loaded value though! :-o
			//!! I think myco::Slider will just round it...

		//
		// Sync the rendered view... (There's only one now)
		//
		//!! NOTE: Could (should?) as well be done via refreshing a "main view"
		//!! UI component instead! But the "main view" is currently not a UI element,
		//!! but a (the) renderer, directly!... :-/
		//!!
		//
		oon_main_view().reset(); //!! Technically this doesn't belong to the UI (but note:
					//!! (the entire rendering is for end-user consumption!).
					//!! Kinda considered part of the model, but with some
					//!! diegetic UI features (like grid lines)!
					//!! The view needs to be updated on load nonetheless!
		//LOGD << " !! !! !! Entity count after load: " << entity_count();
		//LOGD << " !! !! !! Render-shape count after post-load display reset: " << oon_main_view().shape_count();
	}
}

} // namespace OON