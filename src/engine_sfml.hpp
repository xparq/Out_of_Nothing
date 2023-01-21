#ifndef __ENGINE_SFML__
#define __ENGINE_SFML__

#include "cfg.h"

#include "world_sfml.hpp"
#include "renderer_sfml.hpp"
#include "hud_sfml.hpp"
#include "audio_sfml.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <atomic>


//----------------------------------------------------------------------------
class Engine // "Controller"
//----------------------------------------------------------------------------
{
// Config
public:
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!
	static constexpr float CFG_GLOBE_RADIUS = 50000000; // m
	static constexpr float CFG_THRUST_FORCE = 6e34; // N (kg*m/s^2)
	
	static constexpr float CFG_DEFAULT_SCALE = 0.0000008; //! This one also depends very much on the physics!

	static constexpr float CFG_PAN_STEP = 10; // "SFML defaul pixel" :) (Not quite sure yet how it does coordinates...)

// Player-controls (state)
public:

	size_t globe_ndx = 0; // paranoid safety init (see _setup()!)
	size_t clack_sound = 0; // paranoid safety init (see _setup()!)

	struct Thruster {
		float _throttle = 0;
		float throttle(float new_throttle)
		{
			auto prev_throttle = _throttle;
			_throttle = new_throttle;
			return prev_throttle;
		}
		float throttle() const { return _throttle; }
	};
	//!!Thtusters should have vectorized throttles relative to the body orientation,
	//!!which is currently fixed to be identical to the world coordinate system...
	Thruster thrust_up;
	Thruster thrust_down;
	Thruster thrust_left;
	Thruster thrust_right;

public:
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

protected:
	bool _terminated = false;
	bool _paused = false;
	bool _show_huds = true;
	bool _show_help = false;

public:
	auto toggle_pause()  { _paused = !_paused; pause(_paused); }
	auto paused()  { return _paused; }
	virtual void pause(bool state = true) = 0; //!! dumb way to depend on the actual World type...

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	auto toggle_huds()  { _show_huds = !_show_huds; }
	auto toggle_help()  { _show_help = !_show_help; }

//----------------------------------------------------------------------------
// callbacks supported by the World:
public:

	virtual void collide_hook(World* world, World::Body* obj1, World::Body* obj2)
	{
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->v = {0, 0}; // or bounce, or stick to the other body and take its v, or any other sort of interaction.
		//!!...body->p -= ds...;
	}

	virtual void collide_hook(World* world, World::Body* obj1, World::Body* obj2, float distance)
	{
		return collide_hook(world, obj1, obj2);
	}

	virtual void touch_hook(World* world, World::Body* obj1, World::Body* obj2)
	{
	}

	//! High-level, abstract (not just "generic"!) hook for n-body interactions:
	virtual void interaction_hook(World* world, World::Event event, World::Body* obj1, World::Body* obj2, ...)
	{
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->color += 0x3363c3;
	}
};


//----------------------------------------------------------------------------
class Engine_SFML : public Engine
{
friend class Renderer_SFML;

public:
// SFML-specific World-event hooks:
	void touch_hook(World* world, World::Body* obj1, World::Body* obj2) override
	{
		audio.play_sound(clack_sound);
	}

// Internals... -- not quite yet; just allow access for now:
public:
	World_SFML  world;
	Renderer_SFML renderer;
#ifdef HUD_ENABLED
	HUD_SFML    debug_hud;
	HUD_SFML    help_hud;
#endif

#ifdef AUDIO_ENABLE
	Audio_SFML audio;
#else
	Audio_Stub audio;
#endif

	//------------------------------------------------------------------------
	void pause(bool state = true)  override { _paused = state; world.pause(state); }

	void toggle_music() { audio.toggle_music(); }

protected:
	float _SCALE = CFG_DEFAULT_SCALE;
	float _OFFSET_X = 0, _OFFSET_Y = 0;

public: //!!Currently used by the global standalone fn event_loop() directly!
	sf::RenderWindow& window;
//!!was:	sf::RenderWindow* window; // unique_ptr<sf::RenderWindow> window would add nothing but unwarranted complexity here
public:
// Ops
	//! Should be idempotent to tolerate keyboard repeats (which could be disabled, but better be robust)!
	auto up_thruster_start()    { thrust_up.throttle(CFG_THRUST_FORCE); }
	auto down_thruster_start()  { thrust_down.throttle(CFG_THRUST_FORCE); }
	auto left_thruster_start()  { thrust_left.throttle(CFG_THRUST_FORCE); }
	auto right_thruster_start() { thrust_right.throttle(CFG_THRUST_FORCE); }

	auto up_thruster_stop()     { thrust_up.throttle(0); }
	auto down_thruster_stop()   { thrust_down.throttle(0); }
	auto left_thruster_stop()   { thrust_left.throttle(0); }
	auto right_thruster_stop()  { thrust_right.throttle(0); }

	auto pan_up()     { _OFFSET_Y -= CFG_PAN_STEP; }
	auto pan_down()   { _OFFSET_Y += CFG_PAN_STEP; }
	auto pan_left()   { _OFFSET_X -= CFG_PAN_STEP; }
	auto pan_right()  { _OFFSET_X += CFG_PAN_STEP; }
	auto pan_reset()  { _OFFSET_X = _OFFSET_Y = 0; }
	auto pan_center_body(auto body_id) {
		const auto& body = world.bodies[body_id];
		_OFFSET_X = - body->p.x * _SCALE;
		_OFFSET_Y = - body->p.y * _SCALE;
	}
	auto _pan_adjust_after_zoom() {
		//!!??
	}

	auto zoom_in()  { auto factor = 1.25; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}
	auto zoom_out () { auto factor = 0.80; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}

	//------------------------------------------------------------------------
	void event_loop();
	void update_thread_main_loop();
	void updates_for_next_frame();
	void draw();

	//------------------------------------------------------------------------
	size_t add_body(World_SFML::Body&& obj);
	size_t add_body(); // add a random one
	void   remove_body(size_t ndx);
	void   remove_body(); // delete a random one

	//------------------------------------------------------------------------
	void _setup();
#ifdef HUD_ENABLED	
	void _setup_huds();
#endif

//------------------------------------------------------------------------
// Housekeeping
public:
	Engine_SFML(sf::RenderWindow& _window)
	      : window(_window)
#ifdef HUD_ENABLED	
			, debug_hud(_window)
			, help_hud(_window, 10) // left = 10 
#endif
	{
		_setup();
	}
	
};

#endif // __ENGINE_SFML__
