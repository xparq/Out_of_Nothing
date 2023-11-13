#ifndef _OON_SFML_HPP_
#define _OON_SFML_HPP_

#include "OON.hpp"

#include "View/render_sfml.hpp"//!!...
//!!#include <SFML/Window/Event.hpp>
#include "sfw/GUI.hpp"
#include "UI/hud_sfml.hpp"

//============================================================================
class OON_sfml : public OON
{
friend class View::Renderer_SFML;

//------------------------------------------------------------------------
// API Op. Overrides...
//------------------------------------------------------------------------
public:
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

	virtual void pause_hook(bool newstate) override;
	virtual bool load_snapshot(unsigned slot = 1) override;
	//! The generic version of this is enough for now:
	//virtual bool save_snapshot(unsigned slot = 1) override;

	//--------------------------------------------------------------------
	// "Meta" ops: beyond the gameplay ("user", rather than "player" actions):
#ifndef DISABLE_HUD
	auto toggle_huds()  { _show_huds = !_show_huds; }
	auto toggle_help()  { help_hud.active(!help_hud.active()); }
#else
	auto toggle_huds()  {}
	auto toggle_help()  {}
#endif
	void toggle_muting();
	void toggle_music();
	void toggle_sound_fx();
	void toggle_fullscreen();
	void     fps_throttling(bool onoff); // Apply configured FPS limit if true
	unsigned fps_throttling(unsigned fps = (unsigned)-1); // -1 means query mode; std::optional couldn't help omit it altogether


//------------------------------------------------------------------------
// Callback impl. (overrides)...
//------------------------------------------------------------------------
private:
	bool init() override;
	void onResize() override;

	// Model events
	virtual bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2) override;

	// Game control
	virtual void post_zoom_hook(float factor) override;


//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
//!! Migrating to SimApp's backend-specific part:
	void event_loop() override;
	void update_thread_main_loop() override;
	void updates_for_next_frame() override;
	void draw() override;
//!!	void onResize() override;

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
//!!	using OON::OON;
	OON_sfml(int atgc, char** argv);
	OON_sfml(const OON_sfml&) = delete;

//------------------------------------------------------------------------
// Data / Internals...
//------------------------------------------------------------------------
protected:
	View::Renderer_SFML renderer;
#ifndef DISABLE_HUD
//!!	UI::HUD& debug_hud;
//!!	UI::HUD& help_hud;
	UI::HUD_SFML debug_hud;
	UI::HUD_SFML help_hud;
#endif

};

#endif // _OON_SFML_HPP_
