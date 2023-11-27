#ifndef _OON_SFML_HPP_
#define _OON_SFML_HPP_

#include "OON.hpp"

#include "View/render_sfml.hpp"//!!...
//!!#include <SFML/Window/Event.hpp>
#include "sfw/GUI.hpp"
#include "UI/hud_sfml.hpp"

#include <utility> // std::unreachable

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

	size_t add_body(Model::World::Body&& obj) override;
	void   remove_body(size_t ndx) override;

	//--------------------------------------------------------------------
	// Player (gameplay) action overrides:

	void pause_hook(bool newstate) override;
	bool load_snapshot(const char* fname) override;
	//! The generic version of this is enough for now:
	//virtual bool save_snapshot(const char* fname) override;

//------------------------------------------------------------------------
// Callback impl. (overrides)...
//------------------------------------------------------------------------
private:
	void onResize() override;

	// Game control
	virtual void post_zoom_hook(float factor) override; // Resize cached shapes


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
	UI::HUD_SFML timing_hud;
	UI::HUD_SFML debug_hud;
	UI::HUD_SFML help_hud;

	UI::HUD& ui_gebi(HUD_ID which) override { switch (which) {
		case HelpPanel:   return help_hud;
		case TimingStats: return timing_hud;
		case PlayerData:  return debug_hud;
		default: std::unreachable(); // c++23 only; and this is c++600: [[unreachable]];
			//return help_hud; // Dummy, to shut up some compiler warnings
	}}
#endif
};

#endif // _OON_SFML_HPP_
