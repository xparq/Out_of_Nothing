#ifndef _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#define _LKLWSJHEWIOHFSDIUGWGHWRTW2245_

#include "Backend.hpp"
#include "Config.hpp"
#include "Time.hpp"
#include "Model/World.hpp"

//!!... This is gonna be tough to abstract...
#include "UI/Input.hpp"

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
	virtual bool run() = 0;
	virtual void time_step(int /*steps*/) {} // Negative means stepping backward!

	virtual bool poll_and_process_controls() { return false; }

	void pause(bool newstate = true);
	auto paused() const { return time.paused; }
	auto toggle_pause()  { pause(!paused()); }
	virtual void pause_hook(bool /*newstate*/) {} // Pausing might need support/followup

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	      Model::World& world();
	const Model::World& world() const;
	const Model::World& const_world(); // Explicit const World& of non-const SimApp
	void set_world(const Model::World&);

	virtual void update_world(Szim::Seconds Δt) { world().update(Δt, *this); }

	virtual bool save_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	virtual bool load_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	//virtual bool save_snapshot(const char* filename);
	//virtual bool load_snapshot(const char* filename);
	template <typename... X> // This convoluted way is to support multiple types AND have the 3rd arg optional
	std::string snapshot_filename(size_t slot_ndx = 1, const std::string& format = "snapshot_{}.sav", const X... args) {
		return std::vformat(format, std::make_format_args(slot_ndx, args...));
	}

	using Entity = Model::World::Body;
	size_t entity_count() { return const_world().bodies.size(); }
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
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	SimApp(int argc, char** argv);

	SimApp(const SimApp&) = delete;
	virtual ~SimApp() = default;

//------------------------------------------------------------------------
// Data: Abstract (Generic) Model World & View state etc...
//----------------------------------------------------------------------------
public:
	Config cfg;
	Backend& backend;

private: // <- Forcing the use of accessors
	Model::World _world; // See the *world() accessors!
protected://!! This may need the same "rigor", as do some others...
	Model::View view;

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
