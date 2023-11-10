#ifndef _OON_SFML_HPP_
#define _OON_SFML_HPP_

#include "OON.hpp"

#include "View/render_sfml.hpp"
#include "audio_sfml.hpp"

#include "sfw/GUI.hpp"
#include "UI/hud_sfml.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>

//============================================================================
class OON_sfml : public OON
{
friend class View::Renderer_SFML;

//------------------------------------------------------------------------
// API Op. Overrides...
//------------------------------------------------------------------------
public:

	virtual bool run() override;
	virtual void time_step(int steps) override;

	//--------------------------------------------------------------------
	// Config / Setup:
	//------------------------------------------------------------------------
	virtual size_t add_player(Model::World::Body&& obj) override;
	virtual void   remove_player(size_t ndx) override;
	virtual size_t add_body(Model::World::Body&& obj) override;
	virtual void   remove_body(size_t ndx) override;

	//--------------------------------------------------------------------
	// Player (gameplay) action overrides:

	virtual void on_pause_changed(bool newstate) override;
	virtual bool load_snapshot(unsigned slot = 1) override;
	//! The generic version of this is enough for now:
	//virtual bool save_snapshot(unsigned slot = 1) override;

	//--------------------------------------------------------------------
	// "Meta" ops: beyond the gameplay ("user", rather than "player" actions):
#ifndef DISABLE_HUD
	auto toggle_huds()  { _show_huds = !_show_huds; }
	auto toggle_help()  { help_hud.active(!help_hud.active()); }
#endif
	void toggle_muting();
	void toggle_music();
	void toggle_sound_fx();
	void toggle_fullscreen();
	unsigned fps_throttling(unsigned fps = (unsigned)-1); // -1 means query mode; std::optional can't help with omitting it altogether
	void     fps_throttling(bool onoff); // set default FPS if On, or 0.0 if Off


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
	OON_sfml(int argc, char** argv);
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
	View::Renderer_SFML renderer;

public:
#ifndef DISABLE_AUDIO
	Audio_SFML audio;
#else
	Audio_Stub audio;
#endif
};

#endif // _OON_SFML_HPP_
