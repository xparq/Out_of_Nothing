#ifndef __SIMAPP__
#define __SIMAPP__

#include "Model/World.hpp"
#include "misc/rolling_average.hpp"

#include <atomic>

//----------------------------------------------------------------------------
class SimApp // "Controller"
//----------------------------------------------------------------------------
{
protected:
	Model::World world;

// Config
public:
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!

	//!!Move the rest of these to the Model, too, for now:
	//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
	//!!(They will become props initialized from a real config!)
	static constexpr float CFG_THRUST_FORCE = 6e34f; // N (kg*m/s^2)

	static constexpr float CFG_DEFAULT_SCALE = 0.0000005f; //! This one also depends very much on the physics!

	static constexpr float CFG_PAN_STEP = 10; // "SFML defaul pixel" :) (Not quite sure yet how it does coordinates...)

// Player-controls (state)
public:
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

//----------------------------------------------------------------------------
// Base API:
public:
	auto toggle_physics()  { _paused = !_paused; pause_physics(_paused); }
	auto physics_paused()  { return _paused; }
	virtual void pause_physics(bool state = true) { _paused = state; }; //! override to stop the actual world...

//!!virtual size_t add_player(questionable generic config type here
//!!                          -- but such covariance isn't even supported in C++!...) = 0;
	virtual void   remove_player(size_t ndx) = 0; //!this should then be virtual, too (like destructors)

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	virtual Model::World& get_world() /*const*/;
	virtual void set_world(Model::World&);

	virtual void save_snapshot(unsigned slot = 1); // 1 <= slot <= SLOTS_MAX
	virtual void load_snapshot(unsigned slot = 1); // 1 <= slot <= SLOTS_MAX

//----------------------------------------------------------------------------
// Model event hooks (callbacks):
/*
	virtual bool collide_hook(World* w, World::Body* obj1, World::Body* obj2)
	{w, obj1, obj2;
		return false;
	}
*/
	virtual bool collide_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2, float distance)
	{w, obj1, obj2, distance;
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		//!!...body->p -= ds...;
		return false;
	}

	virtual bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2)
	{w, obj1, obj2;
		return false;
	}

	//! High-level, abstract (not just "generic"!) hook for n-body interactions:
	virtual void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
	{w, event, obj1, obj2;
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->color += 0x3363c3;
	}

//------------------------------------------------------------------------
// Housekeeping
public:
	SimApp() {}
	SimApp(const SimApp&) = delete;
	virtual ~SimApp() {}

//------------------------------------------------------------------------
// Data...
protected:
	bool _terminated = false;
	bool _paused = false;
	RollingAverage<5> avg_frame_delay;

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
