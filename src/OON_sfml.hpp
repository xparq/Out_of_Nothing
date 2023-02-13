#ifndef __OON_SFML__
#define __OON_SFML__

#include "OON.hpp"

#include "renderer_sfml.hpp"
#include "hud_sfml.hpp"
#include "audio_sfml.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>

//============================================================================
class OON_sfml : public OON
{
friend class Renderer_SFML;

public:
// Model event hooks (callbacks):
	bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2) override;

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
	size_t add_body(Model::World::Body&& obj);
	void   add_bodies(size_t n);
	void   remove_body(size_t ndx);
	void   remove_body(); // delete a random one
	void   remove_bodies(size_t n);

	size_t add_player(Model::World::Body&& obj); //!! override;
	void   remove_player(size_t ndx) override;

	//------------------------------------------------------------------------
	void _setup();
	void _setup_huds();

	void onResize();

//------------------------------------------------------------------------
// Housekeeping
public:
	OON_sfml();
	OON_sfml(const OON_sfml&) = delete;

//------------------------------------------------------------------------
// Data -- Internals...
protected:
	sf::RenderWindow window;
	sf::Clock clock;
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

#endif // __OON_SFML__
