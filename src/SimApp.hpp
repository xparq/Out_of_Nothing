#ifndef __SIMAPP__
#define __SIMAPP__

#include "Model/World.hpp"
#include "misc/rolling_average.hpp"
import Storage;

#include <atomic>

//============================================================================
class SimApp // "Controller"
{
//----------------------------------------------------------------------------
// Config (static)...
//----------------------------------------------------------------------------
public:
	//!!Move the rest of these to the Model, too, for now:
	//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
	//!!(They will become props initialized from a real config!)
	static constexpr float CFG_THRUST_FORCE = 6e34f; // N (kg*m/s^2)
	static constexpr float CFG_DEFAULT_SCALE = 0.0000005f; //! This one also depends very much on the physics!
	static constexpr float CFG_PAN_STEP = 10; // "SFML defaul pixel" :) (Not quite sure yet how it does coordinates...)
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!

//----------------------------------------------------------------------------
// Base API...
//----------------------------------------------------------------------------
public:
	virtual bool run() = 0;

	auto toggle_physics()  { _paused = !_paused; pause_physics(_paused); }
	auto physics_paused()  { return _paused; }
	virtual void pause_physics(bool state = true) { _paused = state; }; //! override to stop the actual world...

	virtual size_t add_player(Model::World::Body&&) = 0; //!!Questionable "generic config" input type!... ;)
	                //!! Bbut C++ doesn't have the covariance needed here.
	                //!! (Still added this cringy fn. for consistency.)
	virtual void   remove_player(size_t ndx) = 0; //!this should then be virtual, too (like destructors)

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	Model::World const& get_world() const;
	Model::World& get_world();
	void set_world(Model::World const&);
	void set_world(Model::World &);

	virtual bool save_snapshot(unsigned slot = 1); // 1 <= slot <= SLOTS_MAX
	virtual bool load_snapshot(unsigned slot = 1); // 1 <= slot <= SLOTS_MAX

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
// Data / Game (World) State...
//----------------------------------------------------------------------------
protected:
	Model::World world;

//------------------------------------------------------------------------
// Data / Internals...
//----------------------------------------------------------------------------
protected:
	bool _terminated = false;
	bool _paused = false;
	//!!Migrate to the Metrics system:
	RollingAverage<5> avg_frame_delay;

	// Player-controls (transient state)
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

	enum KBD_STATE {
		SHIFT, LSHIFT, RSHIFT,
		CTRL, LCTRL, RCTRL,
		ALT, LALT, RALT,
		CAPS_LOCK, NUM_LOCK, SCROLL_LOCK, // CAPS: 60, but NUM & SCROLL just gives key.code -1 :-/
		__SIZE
	};

	bool kbd_state[KBD_STATE::__SIZE] = {0}; // Can't just be bool, 'coz the doubled modifiers need 2 bits!
};

#endif // __SIMAPP__
