#ifndef __OON_SFML__
#define __OON_SFML__

#include "OON.hpp"

#include "renderer_sfml.hpp"
#include "audio_sfml.hpp"

#include "sfw/GUI.hpp"
#include "UI/hud_sfml.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>

//============================================================================
class OON_sfml : public OON
{
friend class Renderer_SFML;

//------------------------------------------------------------------------
// API Op. Overrides...
//------------------------------------------------------------------------
public:

	virtual bool run() override;

	//--------------------------------------------------------------------
	// Config / Setup:
	//------------------------------------------------------------------------
	virtual size_t add_player(Model::World::Body&& obj) override;
	virtual void   remove_player(size_t ndx) override;
	virtual size_t add_body(Model::World::Body&& obj) override;
	virtual void   remove_body(size_t ndx) override;

	//--------------------------------------------------------------------
	// Player (gameplay) action overrides:

	virtual void pause_physics(bool state = true) override;
	virtual bool load_snapshot(unsigned slot = 1) override;
	//! The generic version of this is enough for now:
	//virtual bool save_snapshot(unsigned slot = 1) override;

	//--------------------------------------------------------------------
	// "Meta" ops: beyond the gameplay ("user", rather than "player" actions):
#ifndef DISABLE_HUD
	auto toggle_huds()  { _show_huds = !_show_huds; }
	auto toggle_help()  { help_hud.active(!help_hud.active()); }
#endif
	void toggle_music() { audio.toggle_music(); }
	void toggle_sound_fxs() { audio.toggle_sound(clack_sound); }
	void toggle_fullscreen();
	bool sw_fps_throttling(int newstate = -1); // -1 means read; std::optional can't help with omitting it altogether


//------------------------------------------------------------------------
// Callback impl. (overrides)...
//------------------------------------------------------------------------
private:
	// Model events
	virtual bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2) override;

	// Game control
	virtual void post_zoom_hook(float factor) override;


//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
	void event_loop();
	void update_thread_main_loop();

	void updates_for_next_frame();
	void draw();

	void onResize();

	void _setup();
	void _setup_UI();

	// Misc...
	// Compensate for zoom displacement when the player object is not centered;
	// called by post_zoom_hook
	void _adjust_pan_after_zoom(float factor); //!! A generic _adjust_pan(), and even _adjust_view() should exist, too, at least for consistency!
		//!! This is not SFML-specific, BTW, but the post_zoom_hook override already is (for using the SFML-dependent renderer),
		//!! so we're stuck here with this, due to that...

//------------------------------------------------------------------------
// C++ mechanics...
//------------------------------------------------------------------------
public:
	OON_sfml();
	OON_sfml(const OON_sfml&) = delete;

//------------------------------------------------------------------------
// Data / Internals...
//------------------------------------------------------------------------
protected:
	sf::RenderWindow window;
	sfw::GUI gui;
#ifndef DISABLE_HUD
	UI::HUD_SFML    debug_hud;
	UI::HUD_SFML    help_hud;
	bool _show_huds = true;
#endif
	sf::Clock clock;
	Renderer_SFML renderer;

#ifndef DISABLE_AUDIO
	Audio_SFML audio;
#else
	Audio_Stub audio;
#endif
};

#endif // __OON_SFML__
