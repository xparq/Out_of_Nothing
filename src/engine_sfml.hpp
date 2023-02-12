#ifndef __ENGINE_SFML__
#define __ENGINE_SFML__

#include "world_sfml.hpp"
#include "renderer_sfml.hpp"
#include "hud_sfml.hpp"
#include "audio_sfml.hpp"

#include "misc/rolling_average.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>

#include <atomic>

//----------------------------------------------------------------------------
class SimApp // "Controller"
//----------------------------------------------------------------------------
{
// Config
public:
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!
	static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
	static constexpr float CFG_THRUST_FORCE = 6e34f; // N (kg*m/s^2)
	
	static constexpr float CFG_DEFAULT_SCALE = 0.0000005f; //! This one also depends very much on the physics!

	static constexpr float CFG_PAN_STEP = 10; // "SFML defaul pixel" :) (Not quite sure yet how it does coordinates...)

// Player-controls (state)
public:

	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

public:
	auto toggle_physics()  { _paused = !_paused; pause_physics(_paused); }
	auto physics_paused()  { return _paused; }
	virtual void pause_physics(bool state = true) { _paused = state; }; //! override to stop the actual world...

//!!virtual size_t add_player(questionable generic config type here
//!!                          -- but such covariance isn't even supported in C++!...) = 0;
	virtual void   remove_player(size_t ndx) = 0; //!this should then be virtual, too (like destructors)

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

//----------------------------------------------------------------------------
// callbacks supported by the World:
public:

/*
	virtual bool collide_hook(World* w, World::Body* obj1, World::Body* obj2)
	{w, obj1, obj2;
		return false;
	}
*/
	virtual bool collide_hook(World* w, World::Body* obj1, World::Body* obj2, float distance)
	{w, obj1, obj2, distance;
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		//!!...body->p -= ds...;
		return false;
	}

	virtual bool touch_hook(World* w, World::Body* obj1, World::Body* obj2)
	{w, obj1, obj2;
		return false;
	}

	//! High-level, abstract (not just "generic"!) hook for n-body interactions:
	virtual void interaction_hook(World* w, World::Event event, World::Body* obj1, World::Body* obj2, ...)
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


//============================================================================
class Engine_SFML : public SimApp
{
friend class Renderer_SFML;

public:
// SFML-specific World-event hooks:
	bool touch_hook(World* w, World::Body* obj1, World::Body* obj2) override;

public:
// Ops
	bool run();

	void pause_physics(bool state = true) override {
		SimApp::pause_physics(state);
		//! Need to start from 0 when unpausing.
		//! (The other reset, on pausing, is redundant, but keeping for simplicity.)
		clock.restart();
	}
	auto toggle_interact_all()  { world._interact_all = !world._interact_all; }

	//! Should be idempotent to tolerate keyboard repeats (which could be disabled, but better be robust)!
	void    up_thruster_start();
	void  down_thruster_start();
	void  left_thruster_start();
	void right_thruster_start();
	void    up_thruster_stop();
	void  down_thruster_stop();
	void  left_thruster_stop();
	void right_thruster_stop();

	auto pan_up()     { _OFFSET_Y -= CFG_PAN_STEP; }
	auto pan_down()   { _OFFSET_Y += CFG_PAN_STEP; }
	auto pan_left()   { _OFFSET_X -= CFG_PAN_STEP; }
	auto pan_right()  { _OFFSET_X += CFG_PAN_STEP; }
	auto pan_reset()  { _OFFSET_X = _OFFSET_Y = 0; }
	void pan_center_body(auto body_id);
	void pan_follow_body(auto body_id, float old_x, float old_y);

	void _pan_adjust_after_zoom() { /* !!?? */ }

	auto zoom_in()  { auto factor = 1.25f; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}
	auto zoom_out () { auto factor = 0.80f; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}

	void toggle_music() { audio.toggle_music(); }
	void toggle_sound_fxs() { audio.toggle_sound(clack_sound); }
#ifndef DISABLE_HUD
	auto toggle_huds()  { _show_huds = !_show_huds; }
	auto toggle_help()  { help_hud.active(!help_hud.active()); }
#endif


	//------------------------------------------------------------------------
	void event_loop();
	void update_thread_main_loop();
	void updates_for_next_frame();
	void draw();

	void toggle_fullscreen();

	//------------------------------------------------------------------------
	void spawn(size_t n = 1);

	//------------------------------------------------------------------------
	// These are low-level ops, not user/player actions!
	size_t add_body(); // add a random one
	size_t add_body(World_SFML::Body&& obj);
	void   add_bodies(size_t n);
	void   remove_body(size_t ndx);
	void   remove_body(); // delete a random one
	void   remove_bodies(size_t n);

	size_t add_player(World_SFML::Body&& obj); //!! override;
	void   remove_player(size_t ndx) override;

	//------------------------------------------------------------------------
	void _setup();
#ifndef DISABLE_HUD
	void _setup_huds();
#endif

//------------------------------------------------------------------------
// Housekeeping
public:
	Engine_SFML();
	Engine_SFML(const Engine_SFML&) = delete;

//------------------------------------------------------------------------
// Data -- Internals...
protected:
	sf::RenderWindow window;
	sf::Clock clock;

	World_SFML  world;
	Renderer_SFML renderer;

#ifndef DISABLE_HUD
	HUD_SFML    debug_hud;
	HUD_SFML    help_hud;
	bool _show_huds = true;
#endif

#ifndef DISABLE_AUDIO
	Audio_SFML audio;
#else
	Audio_Stub audio;
#endif

	float _SCALE = CFG_DEFAULT_SCALE;
	float _OFFSET_X = 0, _OFFSET_Y = 0;

	size_t globe_ndx = 0; // paranoid safety init (see _setup()!)
	size_t clack_sound = 0; // paranoid safety init (see _setup()!)
};

#endif // __ENGINE_SFML__
