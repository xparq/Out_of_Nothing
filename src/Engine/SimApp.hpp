#ifndef _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#define _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#include "_build_cfg.h"

#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?
#include "Backend.hpp"
#include "SimAppConfig.hpp"
#include "Time.hpp"

//!!... The UI and IO etc. are gonna be tough to abstract...
//!!namespace sfw { class GUI; }
#include "sfw/GUI.hpp"//!! REMOVE FROM HERE! (After hiding it behind a ref., those
                      //!! (mostly?) client .cpps that use should include it individually!)
//!!#include "UI/HUD.hpp"
#include "UI/Input.hpp"

#include "Model/World.hpp"
#include "View/ViewPort.hpp"

#include "sz/unilang.hh" // On/Off
#include "sz/counter.hh"
#include "sz/rolling_average.hh"

import Storage;

#include <atomic>
#include <format> // vformat
#include <string>

namespace Szim {

//============================================================================
class SimApp // Universal Sim. App Base ("Engine Controller")
{
//----------------------------------------------------------------------------
// Config...
//----------------------------------------------------------------------------
protected:
	static constexpr auto DEFAULT_CFG_FILE = "default.cfg";

//----------------------------------------------------------------------------
// API...
//----------------------------------------------------------------------------
public:
	bool run();

	virtual bool init() { return true; }
	virtual bool poll_and_process_controls() { return false; } // false: no inputs, nothing to do
	virtual void update_world(Szim::Seconds Δt) { world().update(Δt, *this); }
	virtual void time_step(int /*steps*/) {} // Negative means stepping backward!

	void     toggle_fullscreen();
	void     fps_throttling(bool onoff); // Apply configured FPS limit if true
	unsigned fps_throttling(unsigned fps = (unsigned)-1); // -1 means query mode; std::optional couldn't help omit it altogether

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }
	void pause(bool newstate = true);
	auto paused() const { return time.paused; }
	auto toggle_pause()  { pause(!paused()); }
	virtual void pause_hook(bool /*newstate*/) {} // Pausing might need followup actions in the mechanics

	      Model::World& world();
	const Model::World& world() const;
	const Model::World& const_world(); // Explicit const World& of non-const SimApp
	void set_world(const Model::World&);

	virtual bool save_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	virtual bool load_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	//virtual bool save_snapshot(const char* filename);
	//virtual bool load_snapshot(const char* filename);
	template <typename... X> // This convoluted way is to support all kinds of things that I forgot... :)
	std::string snapshot_filename(size_t slot_ndx = 1, const std::string& format = "{}snapshot_{}.sav", const X... args) {
		return std::vformat(format, std::make_format_args(
			cfg.data_dir, slot_ndx, args...)); //!! if data_dir is not empty, it must have a trailing /
	}

	using Entity = Model::World::Body;
	size_t entity_count() const { return world().bodies.size(); }
	// Thread-safe, slower access:
	      Entity& entity(size_t index)       { return *world().bodies[index]; }
	const Entity& entity(size_t index) const { return *world().bodies[index]; }
	const Entity& const_entity(size_t index) { return *world().bodies[index]; }
	//!! This might be misguided, but keeping it as a reminder...
	// Unprotected, faster access (when already locked):
	      Entity& _entity(size_t index)       { return *_world.bodies[index]; }
	const Entity& _entity(size_t index) const { return *_world.bodies[index]; }
	const Entity& _const_entity(size_t index) { return *_world.bodies[index]; }

	virtual size_t add_player(Model::World::Body&&) = 0; //!!Questionable "generic config" input type!... ;)
	                //!! But C++ doesn't have the covariance needed here.
	                //!! (Still added this cringy fn. for consistency.)
	virtual void   remove_player(size_t ndx) = 0; //!this should then be virtual, too (like destructors)

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
	virtual void draw() = 0;
	virtual void onResize() {}

//------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	SimApp(int argc, char** argv);

	SimApp(const SimApp&) = delete;
	virtual ~SimApp() = default;

//------------------------------------------------------------------------
// Data: Abstract (Generic) Model World & View state etc...
//----------------------------------------------------------------------------
protected:
	Args args;
public://!! E.g. the renderer still needs these...
	SimAppConfig cfg;
	Backend& backend;
protected:
	bool is_fullscreen = false; //!! May need to move it into the backend!
//--------------
//!!
sfw::GUI gui; //!! Forward-declare only, and the backend-specific impl. ctor should create it... somehow... :)
	              //!! -- e.g. via a unique_ptr!
//!!	View::Renderer_SFML renderer; //! The SFML UI has its own, but sharing the same SFML window! :-o

//--------------
private: // <- Forcing the use of accessors
	Model::World _world; // See the *world() accessors!
public://!! Alas, the renderer needs it... 
        //!! Oh, but move the renderer here, and the ViewPort should be part of that!too, BTW! But it's both SFML *and* app dependent yet!... :-/
	View::ViewPort view;

public://!! Still directly set from main, hence the public yet!
	sz::CappedCounter<Szim::CycleCount> iterations; // number of model update cycles (from the start of the main (run) loop, or load; !!TBD)
		// 1 calendar year = 3,784,320,000 cycles at 120 fps, so even 32 bits are quite enough!
		// But for longer runs (on persistent-world servers), or for higher resolutions, let's go 64...

	sz::Counter<int> stepthrough; // # of scheduled steps to advance/retrace, while in halt (paused) mode

//------------------------------------------------------------------------
// Data / Internals...
//----------------------------------------------------------------------------
protected:
	// Workflow/logic control
	bool _terminated = false;

	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

	// Time control
	Time time;
	//!!??Move to the Metrics system and resuse it from there:
//!!	sz::SmoothRollingAverage<0.993f, 1/30.f> avg_frame_delay;
	sz::RollingAverage<30> avg_frame_delay;

}; // class SimApp
} // namespace Szim
#endif // _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
