#include "SimApp.hpp"

#include "View/ScreenView.hpp"

#include "sz/str.hh"
//	using sz::to_bool

#include <string>
	using std::string, std::to_string;
//	using std::stoul, std::stof;
//	using namespace std::string_literals;
//#include <string_view>
//	using std::string_view;
#include "sz/sys/fs.hh"
	using sz::prefix_if_rel;

#include <format>
	using std::format;
#include <iostream>
	using std::cerr, std::cout, std::endl;
#include <thread>
//	using std::thread, std::sleep_for;
#include <chrono>
	using namespace std::chrono_literals;
//#include <stdexcept>
//	using std::runtime_error;

#include "diag/Log.hpp"
#include "sz/DBG.hh" // My old debug macros for DEBUG builds
#include "sz/lang/IGNORE.hh"


namespace Szim {

//============================================================================
void SimApp::show_cmdline_help(const Args& args, const char* banner)
{
	std::string descr;

	if (!banner) {
		banner = "\"Intuition\" - Simulation engine and app. dev. framework\n";
	}

	descr = R"(
Usage:)"; descr += args.exename(); descr += " [options...]\n";
	descr += R"(
Options:

  -h | -? | --help
  	Show this help.
  -V
  	Show version information.
)";

	cout << banner << descr;
}

//----------------------------------------------------------------------------
void SimApp::init()
//
// Internal engine init, called from the ctor...
//
// >>>  NO VIRTUAL DISPATCH IS AVAILABLE HERE YET!  <<<
//
{
	LOGD << "SimApp::init entered...";

	// Guard against multiple calls (which would happen if not overridden!):
	static auto done = false; if (done) return; else done = true; // The exit code may have already been set!

	// Logging...
	// The logger instance(s) has (have) already been through static init, with defaults.
	// Here we just adjust what we can/need, e.g. from command line args etc.
	using namespace diag;

	// Log level override, if requested (with --log-level=<letter>)
	//! NOTE: WAY TOO LATE here for debugging the App ctor init chain (which has already been done)! :-/
	auto log_level = log::letter_to_level(args("log-level")[0]);
	if (log_level) { log::LogMan::instance()->set_level(log_level); }

	//!! Open the file-backed Session Log:
	//!!log::init(log_level, "Szim-debug.log");


	// Apply the config...

	// Misc. fixup that should've been in the ctors, but C++...

	// Some args aren't yet (!!?? can't/shouldn't be?) handled by SimAppConfig itself...

	// UI...
	if (cfg.headless)
		gui.disable();

	// Audio...
	if (cfg.start_muted)
		backend.audio.enabled(false);

	// Time control...
	iterations.max(cfg.iteration_limit);
	if (cfg.fixed_model_dt_enabled)
		time.last_model_Δt = cfg.fixed_model_dt; // Otherwise no one might ever init this...

	// Session pre-init...
	if (!sz::to_bool(args("session-autosave"), sz::str::empty_is_true) // Explicitly set to false?
	    || args["no-session-autosave"] || args["session-no-autosave"]
	    || args["session-no-save"] || args["no-session-save"]) // Also support these "DEPRECATED" options (#556)!
		session.set_autosave(false);
	if (!args("session-save-as").empty()) // Even if autosave disabled. (Could be reenabled later, or manual save...)
		session.set_save_as_filename(args("session-save-as"));

	LOG << "<<< Engine/API initialized. >>>";
	LOGD << "SimApp::init finished.";
}

//----------------------------------------------------------------------------
void SimApp::done()
//
// Internal engine cleanup, called from the dtor...
//
// >>>  NO VIRTUAL DISPATCH IS AVAILABLE HERE ANY MORE!  <<<
//
{
	// Guard against multiple calls (which can happen if not overridden):
	static auto done = false; if (done) return; else done = true; // The exit code may have already been set!

	LOG << "<<< Engine/API shutting down... >>>";
}

//----------------------------------------------------------------------------
int SimApp::run()
{
/*
	//!!
	//!! I'VE JUST DISABLED THIS LEFTOVER CRUFT (SHOULD BE IN THE BACKEND ANYWAY),
	//!! SO THAT I COULD MOVE run() TO THE NON-BACKEND-SPECIFIC PART OF SimApp FINALLY.
	//!! AND THIS CHANGE HAS APPARENTLY BEEN REDUNDANT...
	//!! I'M STILL LEAVING IT HERE, AS THE UNDERLYING ISSUE (#190) REMAINS UNRESOLVED.
	//!
	//! The event loop will block and sleep.
	//! The update thread is safe to start before the event loop, but we'd also need to
	//! draw things before the first event, so we have to release the SFML (OpenGL) Window (crucial!),
	//! and unfreeze the update thread (which would wait on the first event by default).
	//!
	//!! [THINGS HAVE CHANGED SINCE THEN! E.g. events are polled now also in threaded mode,
	//!! so no blocking before the first one. And that "crucial" GL release seems to not
	//!! make any difference whatsoever now... Everything just works as before without it.
	//!! -- BUT THAT MAY BE A LUCKY COINCIDENCE OF THREAD SCHEDULING! :-o
	//!! Also: what does/did the 1st-event "freeze" have to do with setActive()?!]
	//!!
	if (!((SFML_Backend&)backend).SFML_window().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(false) failed, WTF?! Terminating.\n";
		return -1;
	}
*/
	if (terminated()) // Terminated during construction? (e.g. by the default init())...
		return exit_code();

	init(); // Unlike the ctor, this calls the override (or the "onced" NOOP default, if none)

	if (terminated()) // init() may have been "aborted"...
		return exit_code();
		// NOTE: If there's no init() override (-- weird, but who knows! :-o ),
		// but there *is* an overridden done() (-- wow, even weirder!!! :) ),
		// that will be called normally, as if the default init was the client's.

	LOG << "Engine: App initialized, starting main loop...";

	ui_event_state = SimApp::UIEventState::IDLE;

//!!sim_init() {
	// Avoid the accidental Big Bang (#500):
	backend.clock.restart(); //! The clock auto-starts at construction(!), causing a huge initial delay, so reset it!

	// ...but do a controlled Big Bang instead (#504):
	float default_BigBang_InflationInterval_s = 0.3f;
	try { default_BigBang_InflationInterval_s = std::stof(args("initial-dynamic-dt")); } catch(...){}
	float BigBang_InflationInterval_s =
		cfg.get("sim/timing/initial_dynamic_dt", default_BigBang_InflationInterval_s);
DBG_(BigBang_InflationInterval_s);
	std::this_thread::sleep_for(std::chrono::duration<float>(BigBang_InflationInterval_s)); //!! #504: Prelim. (mock/placeholder) "support" for a controlled Big Bang
		//!! This does (should do) nothing for deterministic (fixed-dt) time drive!
		//!! More work is needed to make the Big Bang orthogonal to the timing method!
//!! }


#ifndef DISABLE_THREADS
DBGTRACE;
	std::thread game_state_updates(&SimApp::update_thread_main_loop, this);
		//! NOTES:
		//! - When it wasn't a member fn, the value vs ref. form was ambiguous and failed to compile!
		//! - The thread ctor would *copy* its params (by default), which would be kinda wonky for the entire app. ;)
#endif

	if (!cfg.headless) {
		event_loop();
	} else {
		// Manually loop the updates if no threading:
#ifdef DISABLE_THREADS
//		while (!terminated()) {
//			update_thread_main_loop(); // <- Doesn't actually loop, when threads are disabled, so crank it from here!
//		}
#endif
	}


#ifndef DISABLE_THREADS
//DBG "TRACE - before threads join";
	game_state_updates.join();
#endif

	LOG << "Engine: Main loop finished, cleaning up the app...";

	done(); // Unlike the dtor, this calls the override (or the "onced" NOOP default if none)

	return exit_code();
}


//----------------------------------------------------------------------------
void SimApp::request_exit(int exit_code)
{
	_terminated = true;
	_exit_code = exit_code;
}

//----------------------------------------------------------------------------
void SimApp::pause(bool newstate)
{
	time.paused = newstate;
	pause_hook(newstate);
}

bool SimApp::toggle_pause() { pause(!paused()); return paused(); }

bool SimApp::toggle_fixed_model_dt()
{
	//!!! THREADING !!!

	cfg.fixed_model_dt_enabled = !cfg.fixed_model_dt_enabled;

	//!! + this, but only to support the debug HUD! :) :-o
	if (cfg.fixed_model_dt_enabled) time.last_model_Δt = cfg.fixed_model_dt;

	return cfg.fixed_model_dt_enabled;
}


//----------------------------------------------------------------------------
//!! Updating is not yet (inherently) thread-safe!
//!! These sould be atomic/blocking/mutex-protected/...!
//!! (Delegating that to all the various World::mutators, and imposing deps.
//!! on threading (etc.) mechanics there feels like a terrible idea!)
//!! While update() and load() are called currently from a locked section of the
//!! event loop anyway, it's a crime to rely on just that!
      Model::World& SimApp::world()       { return _world; }
const Model::World& SimApp::world() const { return _world; }
const Model::World& SimApp::const_world() { return _world; }
void SimApp::set_world(Model::World const& w) { _world = w; }


//----------------------------------------------------------------------------
EntityID SimApp::add_entity(Entity&& temp)
{
	return world().add_body(std::forward<decltype(temp)>(temp)); //!!?? That forward is redundant here?
}

EntityID SimApp::add_entity(const Entity& src)
{
	return world().add_body(src);
}

void SimApp::remove_entity(EntityID id)
{
	world().remove_body(id);
}


//----------------------------------------------------------------------------
bool SimApp::quick_save_snapshot(unsigned slot_id) // starting from 1, not 0!
{
/*
	using namespace MEMDB;
	assert(slot_id > 0 && slot_id <= MAX_WORLD_SNAPSHOTS); //!!should become a runtime "filename OK" check

	auto slot = slot_id - 1; //! internally they are 0-based tho...
	decltype(saved_slots) slot_bit = 1 << slot;
	if (saved_slots & slot_bit) {
		cerr << "- WARNING: Overwriting previously saved state at slot #" << slot_id << "!...\n";
	}

	world_snapshots[slot] = world(); // :)
	saved_slots |= slot_bit;
*/
	return save_snapshot(
		snapshot_filename(slot_id, cfg.quick_snapshot_filename_pattern.c_str()).c_str());
}

//----------------------------------------------------------------------------
bool SimApp::quick_load_snapshot(unsigned slot_id) // starting from 1, not 0!
{
/*
	using namespace MEMDB;
	assert(slot_id > 0 && slot_id <= MAX_WORLD_SNAPSHOTS); //!!should become a runtime "filename OK" check

	auto slot = slot_id - 1; //! internally they are 0-based tho...
	decltype(saved_slots) slot_bit = 1 << slot;
	if (! (saved_slots & slot_bit)) {
		cerr << "- WARNING: No saved state at slot #" << slot_id << " yet!\n";
		return false;
	}
	set_world(world_snapshots[slot]);
	cerr << "World state loaded from slot " << slot_id << ".\n";
*/
	return load_snapshot(
		snapshot_filename(slot_id, cfg.quick_snapshot_filename_pattern.c_str()).c_str());
}


//----------------------------------------------------------------------------
void SimApp::player_mark_active(unsigned player_id)
{
	assert(players.size());
	assert(player_id <= players.size()); // <=, not just <, as player_id is 1-based!

	player(player_id).last_action_time = time.real_session_time;
}

float SimApp::player_idle_time(unsigned player_id) const
{
	assert(players.size());
	assert(player_id <= players.size()); // <=, not just <, as player_id is 1-based!

	return time.real_session_time - player(player_id).last_action_time > cfg.player_idle_threshold ?
	       time.real_session_time - player(player_id).last_action_time : 0;
}


//----------------------------------------------------------------------------
bool SimApp::check_if_entity_is_at_viewpos(EntityID id, float x, float y) const // virtual
{
	const auto& e = entity(id);
	//!! Check if view pos is cached first! (But that lookup could be even more expensive... MEASURE!)
	//!! Actually, in OONApp_sfml it is -- make this "tunnelable"!...
	const auto& camera = main_view().camera();
	auto ep = camera.world_to_view_coord(e.p);
	//!! ... = e.bounding_box();
	auto box_R = e.r * camera.scale(); //!! Not a terribly robust method to get that size...
	auto distance = Math::mag2(ep.x - x, ep.y - y); //!! Sigh... #327
//DBG "---> ...checking click at ("<<x<<", "<<y<<") against entity #"<<i<<" at ("<<ep.x<<", "<<ep.y<<")...";

	if (distance <= box_R) {
//DBG "- FOUND entity #" << i;
		return true;
	} else  return false;
}

//----------------------------------------------------------------------------
bool SimApp::entity_at_viewpos(float x, float y, EntityID* entity_id OUT) const // virtual
{
	//!! Assert that EntityID is size_t, or implement proper iteration!...:
	for (size_t i = entity_count(); i-- != 0;) { //!! Poor man's Z-order... Override for less hamfisted ways!
		if (check_if_entity_is_at_viewpos(i, x, y)) {
			*entity_id = i;
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------
void SimApp::undirected_interaction_hook(Model::World* w, Entity* obj1, Entity* obj2, float dt, double distance, ...)
{IGNORE w, obj1, obj2, dt, distance;
}

void SimApp::directed_interaction_hook(Model::World* w, Entity* source, Entity* target, float dt, double distance, ...)
{w, source, target, dt, distance;
}

bool SimApp::collide_hook(Model::World* w, Entity* obj1, Entity* obj2, double distance)
{w, obj1, obj2, distance;
	//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
	//!!...body->p -= ds...;
	return false;
}

bool SimApp::touch_hook(Model::World* w, Entity* obj1, Entity* obj2)
{w, obj1, obj2;
	return false;
}

/*!!UPDATE/DELETE/MOVE NOTE:
// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
//!!The model should also pass the physical property/condition ("event type") that made it think these may interact!
//!!A self-documenting alternative would be calling a matching function for each known such event,
//!!but that might be prohibitively expensive in that tight loop, especiall if most callbacks would
//!!just do nothing.
//!!NOTE: This will anyway change to the objects themselves being notified (not the game "superclass")!
void SimApp::interaction_hook(Model::World* w, Model::World::Event event, Entity* obj1, Entity* obj2, ...)
{w, event, obj1, obj2;
	//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
}
!!*/

//----------------------------------------------------------------------------
void SimApp::toggle_fullscreen()
{
	backend.hci.switch_fullscreen(!main_window().cfg.fullscreen);
	auto width  = main_window_width();
	auto height = main_window_height();

	// Refresh engine state:
	main_view().resize(width, height);

	// Notify the client:
	onResize(width, height);
}

//----------------------------------------------------------------------------
unsigned SimApp::fps_throttling(unsigned new_fps_limit/* = -1u*/)
{
	//!! -> #521

	if (new_fps_limit != unsigned(-1)) { // -1 means get!
	// Set...
		LOGD << "Setting FPS limit to "<<new_fps_limit;
		backend.hci.set_frame_rate_limit(new_fps_limit); // 0: no limit
	}

	// Query...
	return backend.hci.get_frame_rate_limit(); // C++ converts it to false when 0 (no limit)
}

void SimApp::fps_throttling(bool onoff)
{
	fps_throttling(unsigned(onoff ? cfg.fps_limit : 0));
//!!...This would just get stuck with the last value: fps_throttling(unsigned(onoff ? backend.hci.get_frame_rate_limit() : 0));
//!! -> #521
}

} // namespace Szim
