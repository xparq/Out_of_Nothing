#ifndef _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#define _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#include "_build_cfg.h"

//#include "Core.hpp" //!! Try to avoid including it here; it's a heavy one!
#include "Szim/RuntimeContext.hpp"

namespace Szim {
	class SimAppConfig;
	class Backend;
}

#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?
#include "Backend.hpp" // E.g. for convenience accessors of backend components
#include "SimAppConfig.hpp"
#include "Core/SessionManager.hpp"
#include "Time.hpp"
#include "Avatar.hpp" // Fw-decl. is not enough for vector<Avatar>: namespace Szim { class Avatar; }
#include "Player.hpp" // Fw-decl. is not enough for vector<Player>: namespace Szim { class Player; }

//!!... The UI and IO etc. are gonna be tough to abstract...
//!!#include "myco/GUI.hpp"//!! REMOVE FROM HERE! (After hiding it behind a ref., those
//!!                       //!! (mostly client) TUs that use it should include it individually!)
#include "Szim/UI-fw.hpp"
#include "Szim/UI/Input.hpp"

#include "Szim/Model/Meta.hpp"
#include "Szim/Model/World.hpp" //!! Could be fw-declared, if some inline impls. moved to the .cpp!

//#include "View/ScreenView.hpp"
namespace Szim::View { class ScreenView; }

#include "sz/lang/.hh" // IGNORE, ON/OFF, AUTO_CONST, OUT
#include "sz/stats/counter.hh"
#include "sz/stats/rolling_average.hh"

//!!GCC still doesn't like modules:
//!!import Storage;

#include <atomic>
//#include <memory> // unique_ptr
#include <format> // vformat <- !!DITCH IT! Heavy, can throw, and the entire templated
                  // (header-based!) filename construction is just a temp. "clever" shortcut...
#include <string>
#include <string_view>
#include <vector>
#include <cassert>


namespace Szim {

	using EntityID = Model::EntityID;
	using PlayerID = Model::PlayerID;


//============================================================================
class SimApp // Universal Sim. App Base ("Engine Controller")
{
//----------------------------------------------------------------------------
// Config... See SimAppConfig!
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// API...
//----------------------------------------------------------------------------

public:
	// Callbacks to be (re)implemented by the app:
	//! NOTE to App developers: *DO NOT* CALL THESE YOURSELF! The engine will orchestrate them.

	virtual bool show_cmdline_help(const Args& args, const char* description = nullptr); // false: exit the app, true: continue

	virtual bool init()     { return true; } // true: no errors, proceed; false: failure, abort (no run(), no done())
	virtual bool done()     { return true; } // Optional,leanup; true: no errors (to report... not much else to do there!
	virtual bool init_cli() { return true; } // Optional init for CLI-only (bac(h) mode (used for run_cli_batch);)
	                                         // false: skip the CLI run — incl. the built-in "standard" cmdline processing for /? /V etc.!
	virtual bool done_cli() { return true;}  // true: no errors (to report... not much else to do there!)


	virtual void init_world() { __world().init(); } //!!TODO: Called by the default init(), before the 1st update_world().
	virtual void get_control_inputs() {} // The impl. should read inputs and update an (abstracted) controller state from those
	                                     // (well, for its own opaque use, so it's kinda academic... ;) ).
		//!! The passing of time should probably also be just another input! (It very much is an "event" in every system.)
		//!! The advantage of defining it so is that it could simplify freezing time altogether;
		//!! model changes could be skipped (note: time-agnostic control inputs should still trigger actions!),
		//!! and an in-game time freeze could be decoupled from a slightly different, low-(engine-)level "hard pause";
		//!! rendering could be optimized to only deal with "off-band" updates; and also: event recording
		//!! would then automatically cover time logging for replay.
		//!! The disadvantage is that the rather isolated, "pristine" time control system would need to come out
		//!! from its current ivory tower and mingle...
	virtual bool react_to_control_inputs() { return false; } // Calculate/queue actions — no exhaustive model changes here yet!
		// true: actions taken, follow-up needed
		// false: inputs ignored, follow-up not needed (may be used to optimize model updates)
		//!! CURRENTLY UNUSED; NEVER CALLED BY THE ENGINE!
		//!! The engine's main loop is not fine-grained (...rigid?) enough to (micro?)manage this yet(?).
		//!! However, this looks like an idiom/pattern important/practical enough to codify...
	virtual void update_world(Time::Seconds Δt) { __world().update(Δt); }

	// These two are also callbacks (also from the Engine), but NOT to be reimplemented. ;)
	// (They just setup/cleanup internal and pre-digested exported app-specific state, before/after
	// the app-level init/done.)
	bool internal_app_init(); // false: abort the app (don't proceed to init())
	bool internal_app_cleanup(); // false: failed (-> reporting)

	//!!MOVE TO THE ENGINE:
	unsigned fps_throttling(unsigned fps = unsigned(-1));
		// Set or query the FPS limit (the default -1 means query)
		//!! std::optional couldn't help eliminate it altogether

	void fps_throttling(bool newstate);
		// Enable/disable configured FPS limit

	//!!MOVE TO THE ENGINE:
	int run_cli_batch();
	int run();
		// Auto-inits (via calling init()), unless init(...) has been explicitly called
		// already by the app.
		// Returns the intended exit code for the process (!0: error)
		// Note: "exit codes" may not be applicable in every execution model!

	void request_exit(int exit_code = 0); // (The code set by the last request_exit(x) will win.)
	bool terminated() const { return _terminated; }
	int  exit_code() const { return _exit_code; } // The code set by the last request_exit(x), or 0
	int  exit_code(int exit_code) { return _exit_code = exit_code; }

	void pause(bool newstate = true);
	auto paused() const { return time.paused; }
	bool toggle_pause(); // Returns the new state
	virtual void pause_hook(bool /*newstate*/) {} // Pausing might need followup actions in the mechanics

	void toggle_fullscreen();

	bool toggle_fixed_model_dt(); // Returns the new state


	float session_time() const { return time.real_session_time; }
	virtual void time_step(int /*steps*/) {} // Negative means stepping backward!

	virtual Model::Core::World* create_world() = 0; //! Overrides should downcast the retval!
	void set_world(Model::Core::World*&& wptr); // Takes ownership from a ptr rvalue.

	      Model::Core::World& __world()       { return * /*!!static_cast<      Model::Core::World*>!!*/(world_); }
	const Model::Core::World& __world() const { return * /*!!static_cast<const Model::Core::World*>!!*/(world_); }
	const Model::Core::World& __const_world() { return * /*!!static_cast<const Model::Core::World*>!!*/(world_); }


	// Visualizing, rendering...
	//!! Tentative! This "main_view" name is just a placeholder/reminder
	//!! that there can be any kinds of (multiple) views!
	      View::ScreenView& main_view()       { return main_view_; }
	const View::ScreenView& main_view() const { return main_view_; }

	// Session save/load...
	enum SaveOpt { UseDefaults = -1, Raw = 0, Compress = 1 };
//!! Moved to App<>:
//!!	virtual bool save_snapshot(const char* filename, SaveOpt flags = UseDefaults);
//!!	        bool load_snapshot(const char* filename, Model::Core::World*);

	// Hooks:
	virtual void on_snapshot_loaded() {}
	virtual void on_snapshot_saved () {}

	template <typename... X> // This must be a template to support custom patterns + args:
	std::string snapshot_filename(size_t slot_ndx = 1,
		std::string_view pattern = SimAppConfig::DEFAULT_SNAPSHOT_FILE_PATTERN,
		const X... args)
	{
		return std::vformat(pattern, std::make_format_args(slot_ndx, args...));
	}

	// Entities...

	size_t entity_count() const { return __world().entity_count(); }

//!! ADD DEBUG-MODE BOUNDS-CHECKING FOR THESE!

//!!	bool entity_at(model::Math::Vector2f world_pos, EntityID* entity_id OUT) const;
//!!	bool entity_at(model::Math::Vector3f world_pos, EntityID* entity_id OUT) const;
//!!	bool entity_at_viewpos(View::Vector2f view_pos, EntityID* entity_id OUT) const;
	bool entity_at_viewpos(float x, float y, EntityID* entity_id OUT) const;
	virtual bool is_entity_at_viewpos(EntityID entity_id, float x, float y) const = 0;

	/*!!OLD:
	virtual EntityID add_entity(OON::Model::Entity&& temp);     // Move from temporary/template obj.
	virtual EntityID add_entity(const OON::Model::Entity& src); // Copy from obj.
	virtual void remove_entity(EntityID ndx);
	!!*/

/*!!
	using EntityTransform = void(*)(Entity&);
	using EntityTransform_ByIndex = void(*)(EntityID ndx);
	virtual void transform_entity(EntityTransform f) {}
	virtual void transform_entity(EntityTransform_ByIndex f) {}
!!*/

	auto number_of_players() const { return players.size(); }

	Player& player(PlayerID player_id = 1) {
		assert(player_id > 0); // 1-based player IDs
		assert(players.size());
		assert(players.size() >= player_id); //! >=, not >, for player_id is 1-based!
		return players[player_id - 1];
	}
	const Player& player(PlayerID player_id = 1) const { return ((SimApp*)this)->player(player_id); }

	EntityID player_entity_ndx(PlayerID player_id = 1) const {
		auto ndx = player(player_id).entity_ndx;
		assert(entity_count());
		assert(ndx < entity_count());
		return ndx;
	}

	float player_idle_time(  PlayerID player_id = 1) const; // No input for so many seconds (0: busy; gated by cfg.player_idle_threshold)
	bool  player_idle(       PlayerID player_id = 1) const { return player_idle_time(player_id) > 0; }
	void  player_mark_active(PlayerID player_id = 1);

	//----------------------------------------------------------------------------
	// Model event hooks (callbacks)

	virtual void init_world_hook() {} // Called by world.init().

	//----------------------------------------------------------------------------
	//Misc convenience helpers
	auto&       main_window()           { return backend.hci.main_window(); }
	const auto& main_window()     const { return backend.hci.main_window(); }
	unsigned main_window_width()  const { return main_window().cfg.width; }
	unsigned main_window_height() const { return main_window().cfg.height; }

//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
	//!! Migrate the Engine-ish impl. parts and remove the `abstractness` of the ifc.:
	virtual void event_loop() = 0;
	virtual void update_thread_main_loop() = 0;
	virtual void updates_for_next_frame() = 0;

	virtual void onResize(unsigned /*width*/, unsigned /*height*/) {} //!! WTF even is this? :)
	                                                                  //!! I guess window-resize for the full-screen toggle...

	//--------------------
	// Rendering... (See also main_view()!)
	virtual void draw() = 0;
	//!! Render sync. kludge (used e.g. by the general-purpose particle emitter) -> #516...):
		public: virtual void resize_shape(EntityID /*ndx*/, float /*factor*/) {}
		public: virtual void resize_shapes(float /*factor*/) {}

//----------------------------------------------------------------------------
// C++ mechanics...
//
// - Note: the Engine tries to not impose/assume most of it, so anything C++
//   specific is internal business of the app implementation.
//----------------------------------------------------------------------------
public:
	SimApp(const RuntimeContext& runtime, int argc, char** argv, View::ScreenView& main_view);
	virtual ~SimApp();

	// No copy (e.g. for the raw World ptr)!
	SimApp(const SimApp&) = delete;
	SimApp& operator=(const SimApp&) = delete;

//----------------------------------------------------------------------------
// Data...
//----------------------------------------------------------------------------
public: //!!TODO: finish the migration to Szim/RuntimeContext!
	RuntimeContext runtime;

protected:
	Args& args;

public: // E.g. the renderer still needs these...
	SimAppConfig& cfg;
	Backend& backend;

	//--------------------------------------------------------------------
	// Engine-specific UI that the client app is also free to use
	// The sim/app "content" has its own rendering, most likely its own UI too,
	// but usually sharing the same (currently: SFML) window!
//protected: <- OONController needs `gui` to skip key polling if the UI has the focus...
	myco::GUI& gui; //!! Forward-declare only, and the backend-specific impl. ctor should create it... somehow... :)
	              //!! -- e.g. via a unique_ptr, or just a plain manual PIMPL. (Plus a gui() accessor then?!)

	//--------------------------------------------------------------------
	// Abstract (Generic) Model World & View state...

protected:
	Model::Core::World* world_ = nullptr; // See the __world() accessors!
	                      // (I'm fucking done with unique_ptr, BTW... Also, the C-like scripting API may thank me for that.)

	View::ScreenView& main_view_;

	Core::SessionManager session;

	//--------------------------------------------------------------------
	// Time control...

	Time::Control time;
	sz::stats::CappedCounter<Szim::Time::CycleCount> iterations; // number of model update cycles (from the start of the main (run) loop, or load; !!TBD)
		// 1 calendar year = 3,784,320,000 cycles at 120 fps, so even 32 bits are quite enough!
		// But for longer runs (on persistent-world servers), or for higher resolutions, let's go 64...
	sz::stats::Counter<int> timestepping; // # of scheduled steps to advance/retrace, while in halt (paused) mode
	//!!??Move to the Metrics system and resuse it from there:
	sz::stats::SmoothRollingAverage<0.991f, 1/30.f> avg_frame_delay;
//	sz::stats::RollingAverage<30> avg_frame_delay;

	//--------------------------------------------------------------------
	// Workflow control...

	bool _terminated = false;
	int  _exit_code = 0;
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // Atomic enums: https://stackoverflow.com/a/23063862/1479945

	//--------------------------------------------------------------------
	// Player support...
public: //!! <- For the View classes :-/
	std::vector<Avatar> avatars;
	std::vector<Player> players; //!!UNUSED YET, JUST FOR COMPILATION TESTING!!

}; // class SimApp
} // namespace Szim
#endif // _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
