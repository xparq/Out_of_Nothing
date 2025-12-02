//!! NEW: #include "Szim/App/Base.hpp"
//!! LEGACY:
#include "Szim/SimApp.hpp"
#include "Szim/View/ScreenView.hpp"
#include "Szim/UI.hpp" // Let toggle_fullscreen() send the news to the UI!

#include "sz/str.hh"
//	using sz::to_bool

#include <string>
	using std::string, std::to_string;
//	using std::stoul, std::stof;
//	using namespace std::string_literals;
//#include <string_view>
//	using std::string_view;
#include "sz/sys/fs.hh"
	using sz::fs::prefix_by_intent;

//#include <format>
//	using std::format;
#include <iostream>
	using std::cout, std::endl; // For normal user-facing output
#include <thread>
//	using std::thread, std::sleep_for;
#include <chrono>
	using namespace std::chrono_literals;
//#include <stdexcept>
//	using std::runtime_error;

#include "Szim/diag/Error.hpp"
#include "Szim/diag/Log.hpp"
#include "sz/DBG.hh" // My old debug macros for DEBUG builds
#include "sz/lang/IGNORE.hh"


// LAST_COMMIT_HASH is defined here:
#include "commit_hash.inc" //! The build proc. is expected to put it on the INCLUDE path.


namespace Szim {

//============================================================================
//----------------------------------------------------------------------------
// SimApp ctor.
//
// NOTE: Most init must be done in the member init list, because the `backend`
//      member is polymorphic, and our only chance to set it to the proper type
//      is there! And it requires prior init. of the config, too. Which requires
//      `args` to be initialized as well. And then, if we are at it, I just put
//      the GUI init there, too, for good measure...
//      (The ctor still has work left to do, so its body is not empty though.)
//
SimApp::SimApp(const RuntimeContext& rt, int argc, char** argv, View::ScreenView& main_view)
	: runtime(rt)
	, args(rt.args)
	, cfg(rt.syscfg) //!! <- .cfg is still the engine config, despite its old, confusing name!
	, backend(rt.backend)
	, gui(rt.gui)
//	, world_(...) // App<> will create it!
//!!TODO: [Future me: TODO WHAT?!...]
	, main_view_(main_view)
//!!	, renderer{View/*!!Not really?...*/::Renderer_SFML::create(main_window())}
	, session(*this/*!!, args("session")!!*/)
{
/*!!
	// Relative paths will be rooted to the dir of 'cfgfile' by default,
	// i.e. unless it's specifically set in the config
	//!!Move to unilang:
	cfg.select(cfgfile);
	//!!auto basename = fs::path(cfgfile).filename().string();
!!*/
}

//----------------------------------------------------------------------------
SimApp::~SimApp()
{
  try { // Let's survive our last moments... :) (Albeit our internal done() is just about empty now...)
	this->SimApp::done(); // Our own internal done() is called "secretly", even if overridden...
	                      // (Note: the qualifier is only for emphasis; dtors don't dispatch virtuals.)

	if (world_) delete world_; //!!Move this to done(), I guess!
  } catch (...) {
	Bug("*REALLY UNEXPECTED* exception from SimApp::done()! :-o ");
	//... throw; // <- Could be useful to see the fireworks in DEBUG mode,
	             //    but can't compile without noexcept-violation warnings.
  }
}

//============================================================================
//----------------------------------------------------------------------------
bool SimApp::internal_app_init()
//
// Internal engine init, called from the ctor...
//
// >>>  NO VIRTUAL DISPATCH IS AVAILABLE HERE YET!  <<<
//
{
	LOGD << "SimApp::internal_app_init entered...";

	// Guard against multiple calls (which could happen if *not* overridden: once from the ctor,
	// and then again from run(), or if the app (main) calls it explicitly):
	static auto done = false; if (done) return true; else done = true; // The exit code may have already been set!

	//
	// Apply the config...
	//
	// - Some args aren't yet (!!?? can't/shouldn't be?) handled by SimAppConfig itself.
	// - Misc. fixup...
	//

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

	//!!---------------------------------------------
	//!!This is custom app-specific init actually!... (Moved here from run(), because that in turn is being replaced by Engine::__run...) {
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
	//!!---------------------------------------------


	LOGD << "SimApp::internal_app_init finished.";
	return true;
}

//----------------------------------------------------------------------------
bool SimApp::internal_app_cleanup()
{
	LOGD << "SimApp::internal_app_cleanup...";
	return true;
}


//============================================================================
bool SimApp::show_cmdline_help(const Args& args, const char* banner)
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
	return false;
}

//----------------------------------------------------------------------------
// Process the cmdline for "actionable" items...
int SimApp::run_cli_batch()
{
	//!! But taking actual actions should be done *after* normal app init, though! :-o
	if (args["?"] || args["h"] || args["help"]) {
		if (!show_cmdline_help(args)) { //! false here means "agree to exit"!
			request_exit(0);
		}
	} else if (args["V"]) {
		cout //! This is not a logging feature, that's why it writes directly to stdout.
			<< "Version: " << LAST_COMMIT_HASH
//!! The build proc. should just send the VDIR tag in a macro! -> #254
#ifdef DEBUG
			<< "-DEBUG"
#endif
#ifdef SFML_STATIC
			<< "-SFML_STATIC"
#endif
			<< '\n';
		//request_exit(0);
	}

	return exit_code();
}

//============================================================================
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
	ui_event_state = SimApp::UIEventState::IDLE;

#ifndef DISABLE_THREADS //!!?? Could these be runtime flags instead (at least here, in run())?
                        //!!   Not sure, because DISABLE_THREADS might also control build options!
DBGTRACE;
	std::thread game_state_updates(&SimApp::update_thread_main_loop, this);
		//! NOTES:
		//! - When it wasn't a member fn, the value vs ref. form was ambiguous and failed to compile!
		//! - The thread ctor would *copy* its params (by default), which would be kinda wonky for the entire app. ;)
#endif

	if (!cfg.headless) { //!! This needs a more systematic treatment than being scattered all across everywhere...
	                     //!! E.g. the event queue shuld still be left running, to allow it to be fed with automated
			     //!! fake events, and should be easy to non-interactively exit that way, too, in addition to
			     //!! the current cycle count limiting cmdline option.
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

void SimApp::set_world(Model::Core::World*&& wptr) { assert(wptr); world_ = wptr; }
/*!!
      Model::Core::World& SimApp::__world()       { return *static_cast<      Model::Core::World*>(world_); }
const Model::Core::World& SimApp::__world() const { return *static_cast<const Model::Core::World*>(world_); }
const Model::Core::World& SimApp::__const_world() { return *static_cast<const Model::Core::World*>(world_); }
!!*/


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
bool SimApp::entity_at_viewpos(float x, float y, EntityID* entity_id OUT) const
{
	//!! Assert that EntityID is size_t, or implement proper iteration!...:
	for (size_t i = entity_count(); i-- != 0;) { //!! Poor man's Z-order... Override for less hamfisted ways!
		if (is_entity_at_viewpos(i, x, y)) {
			*entity_id = i;
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------
void SimApp::toggle_fullscreen()
{
	backend.hci.switch_fullscreen(!main_window().cfg.fullscreen);
	auto width  = main_window_width();
	auto height = main_window_height();

	// Refresh engine state:
	main_view().resize(width, height);

	// Notify the UI (the client app might set up a UI callback if interested in this directly):
	using namespace myco::event;
	gui.dispatch(
		Event<WindowResized>(WindowChange{ .size = {width, height} })
	);

	//!! LEGACY:
	// Notify the client app directly:
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
