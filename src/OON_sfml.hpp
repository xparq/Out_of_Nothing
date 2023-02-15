#ifndef __OON_SFML__
#define __OON_SFML__

#include "OON.hpp"

#include "renderer_sfml.hpp"
#include "UI/hud_sfml.hpp"
#include "audio_sfml.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>

//============================================================================
class OON_sfml : public OON
{
friend class Renderer_SFML;

//------------------------------------------------------------------------
// Model event hooks (callbacks)...
//------------------------------------------------------------------------
public:
	bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2) override;

//------------------------------------------------------------------------
// API Ops...
//------------------------------------------------------------------------
public:
	bool run();

	void pan_center_body(auto body_id);
	void pan_follow_body(auto body_id, float old_x, float old_y);

	auto zoom_in()  { auto factor = 1.25f; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}
	auto zoom_out () { auto factor = 0.80f; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}

#ifndef DISABLE_HUD
	auto toggle_huds()  { _show_huds = !_show_huds; }
	auto toggle_help()  { help_hud.active(!help_hud.active()); }
#endif

	void toggle_music() { audio.toggle_music(); }
	void toggle_sound_fxs() { audio.toggle_sound(clack_sound); }

	void toggle_fullscreen();

	bool sw_fps_throttling(int newstate = -1); // -1 means read; std::optional can't help with omitting it altogether

	//------------------------------------------------------------------------
	void pause_physics(bool state = true) override {
		SimApp::pause_physics(state);
		//! Need to start from 0 when unpausing.
		//! (The other reset, on pausing, is redundant, but keeping for simplicity.)
		clock.restart();
	}

	void spawn(size_t n = 1);
	void exhaust_burst();

	virtual bool load_snapshot(unsigned slot = 1) override;

	//------------------------------------------------------------------------
	// These are low-level ops, not meant to be user/player actions!
	size_t add_body(); // add a random one
	size_t add_body(Model::World::Body&& obj);
	void   add_bodies(size_t n);
	void   remove_body(size_t ndx);
	void   remove_body(); // delete a random one
	void   remove_bodies(size_t n = -1); // -1 -> all

	size_t add_player(Model::World::Body&& obj); //!! override;
	void   remove_player(size_t ndx) override;

//------------------------------------------------------------------------
// Housekeeping...
//------------------------------------------------------------------------
protected:

	void event_loop();
	void update_thread_main_loop();
	void updates_for_next_frame();

	void draw();
	void onResize();

	//------------------------------------------------------------------------
	void _setup();
	void _setup_UI();

public:
	OON_sfml();
	OON_sfml(const OON_sfml&) = delete;

//------------------------------------------------------------------------
// Data / Internals...
//------------------------------------------------------------------------
protected:
	sf::RenderWindow window;
	sf::Clock clock;
	Renderer_SFML renderer;

#ifndef DISABLE_HUD
	UI::HUD_SFML    debug_hud;
	UI::HUD_SFML    help_hud;
	bool _show_huds = true;
#endif

#ifndef DISABLE_AUDIO
	Audio_SFML audio;
#else
	Audio_Stub audio;
#endif
};

#endif // __OON_SFML__
