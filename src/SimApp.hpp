#ifndef __SIMAPP__
#define __SIMAPP__

#include "Model/World.hpp"
#include "UI/Input.hpp"
#include "misc/rolling_average.hpp"
import Storage;

#include <atomic>
#include <format> // vformat

//============================================================================
class SimApp // "Controller"
{
//----------------------------------------------------------------------------
// Config (static)...
//----------------------------------------------------------------------------
protected:
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

	virtual bool poll_and_process_controls() { return false; }

	auto toggle_pause_physics()  { _paused = !_paused; pause_physics(_paused); }
	auto physics_paused()  { return _paused; }
	virtual void pause_physics(bool state = true) { _paused = state; }; //! override to stop the actual world...

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
	SimApp() = default;
	SimApp(const SimApp&) = delete;
	virtual ~SimApp() = default;

//------------------------------------------------------------------------
// Data: Abstract (Generic) Game World (+ View) State...
//----------------------------------------------------------------------------
protected:
	Model::World world;
	Model::View view;

//------------------------------------------------------------------------
// Data / Internals...
//----------------------------------------------------------------------------
protected:
	bool _terminated = false;
	bool _paused = false;
	//!!Migrate to the Metrics system:
	misc::RollingAverage<50> avg_frame_delay;

	// Player-controls (transient state)
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945
};

#endif // __SIMAPP__
