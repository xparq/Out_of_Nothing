#ifndef _SIMAPP_HPP_
#define _SIMAPP_HPP_

#include "Model/World.hpp"
#include "UI/Input.hpp"
#include "sz/unilang.hh" // On/Off
#include "sz/counter.hh"
#include "sz/rolling_average.hh"

import Storage;

#include <atomic>
#include <format> // vformat
#include <string>

//============================================================================
class SimApp // "Controller"
{
//----------------------------------------------------------------------------
// Config (static)...
//----------------------------------------------------------------------------
protected:
	static constexpr auto DEFAULT_CFG_FILE = "default.cfg";
	//!!Move the rest of these to the Model, too, for now:
	//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
	//!!(They will become props initialized from a real config!)
	static constexpr float CFG_THRUST_FORCE = 6e34f; // N (kg*m/s^2)
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!

//----------------------------------------------------------------------------
// Base API...
//----------------------------------------------------------------------------
public:
	virtual bool run() = 0;
	virtual void time_step(int /*steps*/) {} // Negative means stepping backward!

	virtual bool poll_and_process_controls() { return false; }

	void pause(bool newstate = true);
	auto paused() const { return _time_paused; }
	auto toggle_pause()  { pause(!paused()); }
	virtual void on_pause_changed(bool /*newstate*/) {} // Pausing might need support/followup

	virtual size_t add_player(Model::World::Body&&) = 0; //!!Questionable "generic config" input type!... ;)
	                //!! But C++ doesn't have the covariance needed here.
	                //!! (Still added this cringy fn. for consistency.)
	virtual void   remove_player(size_t ndx) = 0; //!this should then be virtual, too (like destructors)

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	Model::World const& get_world() const;
	Model::World& get_world();
	void set_world(Model::World const&);
	void set_world(Model::World &);

	virtual bool save_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	virtual bool load_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	//virtual bool save_snapshot(const char* filename);
	//virtual bool load_snapshot(const char* filename);
	template <typename... X>
	std::string snapshot_filename(size_t slot_ndx = 1, const std::string& format = "snapshot_{}.sav", const X... args) {
		return std::vformat(format, std::make_format_args(slot_ndx, args...));
	}

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
	SimApp(const char* cfgfile = "");

	SimApp(const SimApp&) = delete;
	virtual ~SimApp() = default;

//------------------------------------------------------------------------
// Data: Abstract (Generic) Model World + View State, etc...
//----------------------------------------------------------------------------
protected:
	struct Config //!! Weird, ehh? :) Just a reminder to get real instead...
	{
		std::string cfg_dir;
		std::string asset_dir;
	} cfg;

	Model::World world; // -> get_/set_world()
	Model::View view;

public:
	sz::CappedCounter<uint64_t> iterations; // # of model updates (from the start of the main (run) loop, or load, etc.)
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
	bool  _time_paused = false;
	bool  _time_reversed = false;
	float _time_scale = 1.0f; // > 0 (Decoupled from reversal, for more flexible controls.)
	float last_frame_delay;
	//!!??Move to the Metrics system and resuse it from there:
	sz::RollingAverage<30> avg_frame_delay;
};

#endif // _SIMAPP_HPP_
