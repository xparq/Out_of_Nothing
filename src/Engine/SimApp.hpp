﻿#ifndef _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#define _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#include "_build_cfg.h"

#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?
#include "Backend.hpp"
#include "SimAppConfig.hpp"
#include "SessionManager.hpp"
#include "Time.hpp"

//!!... The UI and IO etc. are gonna be tough to abstract...
//!!namespace sfw { class GUI; }
#include "sfw/GUI.hpp"//!! REMOVE FROM HERE! (After hiding it behind a ref., those
                      //!! (mostly?) client .cpps that use should include it individually!)
//!!#include "UI/HUD.hpp"
#include "UI/Input.hpp"
#include "Model/World.hpp"
//#include "View/ScreenView.hpp"
namespace Szim::View { class ScreenView; }

#include "sz/unilang.hh" // ON/OFF, AUTO_CONST, OUT
#include "sz/counter.hh"
#include "sz/rolling_average.hh"

import Storage;

#include <atomic>
#include <format> // vformat
#include <string>
#include <string_view>

namespace Szim {

//============================================================================
class SimApp // Universal Sim. App Base ("Engine Controller")
{
//----------------------------------------------------------------------------
// Config...
//----------------------------------------------------------------------------
protected:
	static constexpr auto DEFAULT_CFG_FILE = "default.cfg";

	// See also: SimAppConfig

//----------------------------------------------------------------------------
// API...
//----------------------------------------------------------------------------
public:
	//!!virtual... -> #348
	static auto show_cmdline_help(const Args& args, const char* description = nullptr) -> void;

	int run(); // Returns the intended exit code for the process (!0: error)
	           // Note: "exit codes" may not be applicable in every execution model!
	virtual void init(); // Call request_exit(exit_code) to mark it "aborted".
	                     // No need to call the "upstream" init() from an override.
	virtual void done(); // Optional cleanup; will not be called if init() was aborted.
	                     // No need to call the "upstream" done() from an override.
	virtual void poll_controls() {}
	virtual bool perform_control_actions() { return false; } // false: no changes to the model
	virtual void update_world(Time::Seconds Δt) { world().update(Δt, *this); }

	unsigned fps_throttling(unsigned fps = (unsigned)-1);
		// Set or query the FPS limit (the default -1 means query)
		//!! std::optional couldn't help eliminate it altogether

	void fps_throttling(bool newstate);
		// Enable/disable configured FPS limit

	void request_exit(int exit_code = 0); // (The code set by the last request_exit(x) will win.)
	bool terminated() const { return _terminated; }
	int  exit_code() const { return _exit_code; } // The code set by the last request_exit(x), or 0
	//!!TBD: Setting it directly without request_exit(x):
	//int|void exit_code(int exit_code) { return _exit_code = exit_code; }

	void pause(bool newstate = true);
	auto paused() const { return time.paused; }
	bool toggle_pause(); // Returns the new state
	virtual void pause_hook(bool /*newstate*/) {} // Pausing might need followup actions in the mechanics

	void toggle_fullscreen();

	bool toggle_fixed_model_dt(); // Returns the new state

	virtual void time_step(int /*steps*/) {} // Negative means stepping backward!

	      Model::World& world();
	const Model::World& world() const;
	const Model::World& const_world(); // Explicit const World& of non-const SimApp (to spare a cast)
	void set_world(const Model::World&);

	// Visualizing, rendering...
	//!! Tentative! This "main_view" name is just a placeholder/reminder
	//!! that there can be any kinds of (multiple) views!
	      View::ScreenView& main_view()       { return _main_view; }
	const View::ScreenView& main_view() const { return _main_view; }

	// Session save/load...
	virtual bool save_snapshot(const char* filename);
	virtual bool load_snapshot(const char* filename);
	bool quick_save_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	bool quick_load_snapshot(unsigned slot = 1); // See cfg.quick_snapshot_filename_pattern!
	template <typename... X> // This must be a template to support custom patterns + args:
	std::string snapshot_filename(size_t slot_ndx = 1,
		std::string_view pattern = SimAppConfig::DEFAULT_SNAPSHOT_FILE_PATTERN,
		const X... args)
	{
		return std::vformat(pattern, std::make_format_args(slot_ndx, args...));
	}

	// Entities...
	using Entity = Model::World::Body;
	size_t entity_count() const { return world().bodies.size(); }
//!! ADD DEBUG-MODE BOUNDS-CHECKING FOR THESE!
	// Thread-safe, slower access:
	      Entity& entity(size_t index)       { return *world().bodies[index]; }
	const Entity& entity(size_t index) const { return *world().bodies[index]; }
	const Entity& const_entity(size_t index) { return *world().bodies[index]; }
	//!! This might be misguided, but keeping it as a reminder...
	// Unprotected, faster access (when already locked):
	      Entity& _entity(size_t index)       { return *_world.bodies[index]; }
	const Entity& _entity(size_t index) const { return *_world.bodies[index]; }
	const Entity& _const_entity(size_t index) { return *_world.bodies[index]; }

//!!	bool entity_at(model::Math::Vector2f world_pos, size_t* entity_id OUT) const;
//!!	bool entity_at(model::Math::Vector3f world_pos, size_t* entity_id OUT) const;
//!!	bool entity_at_viewpos(View::Vector2f view_pos, size_t* entity_id OUT) const;
	virtual bool entity_at_viewpos(float x, float y, size_t* entity_id OUT) const;
	virtual bool is_entity_at_viewpos(size_t entity_id, float x, float y) const;

	virtual unsigned add_player(Model::World::Body&&) = 0; //!!Questionable "generic config" input type!... ;)
	                //!! (This can't really be done in C++ properly, without RTTI etc.
	                //!! Still added this cringy fn. for consistency & as a reminder.)
	virtual void   remove_player(unsigned player_id) = 0; //!this should then be virtual, too (like destructors)
	virtual       Entity& player_entity(unsigned player_id = 1) = 0;
	virtual const Entity& player_entity(unsigned player_id = 1) const = 0;

	//----------------------------------------------------------------------------
	// Model event hooks (callbacks)
	/*
	virtual bool collide_hook(World* w, World::Body* obj1, World::Body* obj2)
	{w, obj1, obj2;
		return false;
	}
	*/
	virtual bool collide_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2, float distance);
	virtual bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2);

	// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
	// `event` represents the physical property/condition that made it think these might interact.
	//!!NOTE: This will change to the objects themselves being notified (not the game "superclass")!
	virtual void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...);

//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
	//!! Migrate the Engine-ish impl. parts here and remove the `abstractness`:
	virtual void event_loop() = 0;
	virtual void update_thread_main_loop() = 0;
	virtual void updates_for_next_frame() = 0;
	virtual void onResize(unsigned /*width*/, unsigned /*height*/) {}
	// Rendering... (See also main_view()!)
	virtual void draw() = 0;

//----------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	SimApp(int argc, char** argv, View::ScreenView& main_view);
	virtual ~SimApp();

	SimApp(const SimApp&) = delete;

//----------------------------------------------------------------------------
// Data...
//----------------------------------------------------------------------------
protected:
	Args args;
public://!! E.g. the renderer still needs these...
	SimAppConfig cfg;

	Backend& backend;
	protected: bool is_fullscreen = false; //!! Move to the backend!

//--------------
protected:
//!!
	sfw::GUI gui; //!! Forward-declare only, and the backend-specific impl. ctor should create it... somehow... :)
	              //!! -- e.g. via a unique_ptr!
//!!	View::Renderer_SFML renderer; //! The SFML UI has its own, but sharing the same SFML window! :-o
//--------------

	//--------------------------------------------------------------------
	// - Abstract (Generic) Model World & View state...

	SessionManager session;

private: // <- Forcing the use of accessors
	Model::World _world; // See the *world() accessors!

	View::ScreenView& _main_view;

protected:
	//--------------------------------------------------------------------
	// - Time control...

	Time::Control time;
	sz::CappedCounter<Szim::Time::CycleCount> iterations; // number of model update cycles (from the start of the main (run) loop, or load; !!TBD)
		// 1 calendar year = 3,784,320,000 cycles at 120 fps, so even 32 bits are quite enough!
		// But for longer runs (on persistent-world servers), or for higher resolutions, let's go 64...
	sz::Counter<int> timestepping; // # of scheduled steps to advance/retrace, while in halt (paused) mode
	//!!??Move to the Metrics system and resuse it from there:
	sz::SmoothRollingAverage<0.991f, 1/30.f> avg_frame_delay;
//	sz::RollingAverage<30> avg_frame_delay;

	//--------------------------------------------------------------------
	// - Workflow control...

	bool _terminated = false;
	int  _exit_code = 0;
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

}; // class SimApp
} // namespace Szim
#endif // _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
