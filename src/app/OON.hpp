#ifndef __OON__
#define __OON__

#include "Engine/SimApp.hpp"
namespace UI { class HUD; }

#include <cassert>

//============================================================================
class OON : public Szim::SimApp
{
//----------------------------------------------------------------------------
// Config/Setup...
//----------------------------------------------------------------------------
protected:
	void init() override;
	void done() override;

	enum HUD_ID { HelpPanel, TimingStats, PlayerData, };
	virtual UI::HUD& ui_gebi(HUD_ID which) = 0; // get_element_by_id(...)
	bool _show_huds = true;
	void _setup_UI();

//----------------------------------------------------------------------------
// Operations...
//----------------------------------------------------------------------------
public:
	//--------------------------------------------------------------------
	// Player actions...

	// Gameplay...

	virtual void spawn(size_t parent_ndx = 0, size_t n = 1);       //!! require: 0 == player_entity_ndx()
	virtual void exhaust_burst(size_t entity = 0, size_t n = 50); //!! require: 0 == player_entity_ndx()

	auto interact_all(bool state = true)  { world()._interact_all = state; }
	auto toggle_interact_all()  { interact_all(!const_world()._interact_all); }

	//!!These should be idempotent, to tolerate keyboard repeats (which could be disabled but may be problematic)!
	//!!Also -> #105: sanitize thrusters...
	void    up_thruster_start();
	void  down_thruster_start();
	void  left_thruster_start();
	void right_thruster_start();
	void    up_thruster_stop();
	void  down_thruster_stop();
	void  left_thruster_stop();
	void right_thruster_stop();

	// View control (note: panning uses view coordinates!)...

	void pan(Math::Vector2f delta);
	void pan_x(float delta);
	void pan_y(float delta);
	void pan_reset();
	void zoom(float factor); // Multiply the current scaling with 'factor'
	bool scroll_locked();    // Auto-panning via pinned focus point or object
	// - Change the zoom ratio by 'amount' (e.g. 5%):
	void zoom_in(float amount);
	void zoom_out(float amount);
	void zoom_reset(float factor = 0); // If !0, also change the original level by 'factor'!

	// - Poll view controls & adjust:
	bool view_control(float mousewheel_delta = 0); //!! override;
		// Returns true if view adjustments have been requested/performed.
		// Note: this is irrespective of modelling, and is also enabled while paused.
	// - view_control() calls these:
	bool pan_control(); //!!override
	bool zoom_control(float mousewheel_delta = 0); //!!override

	void center_to_entity(size_t id);
	void center_to_player(unsigned player_id = 1);
	void follow_entity(size_t id);
	void follow_player(unsigned player_id = 1);

	//!! A more generic _adjust_pan() (or even _adjust_view()) should also exist, e.g.
	//!! to bring back things on-screen, if drifted off!

	//--------------------------------------------------------------------
	// "Meta" controls (not gameplay/player, but admin/user actions)...
	void toggle_muting();
	void toggle_music();
	void toggle_sound_fx();
#ifndef DISABLE_HUD
	void toggle_huds();
	bool huds_active();
	void toggle_help();
#else
	auto toggle_huds()  {}
	auto huds_active()  { return false; }
	auto toggle_help()  {}
#endif

	//--------------------------------------------------------------------
	// Internals: not even user actions (Well, some still are, basically for testing.)
	//!!Make a proper distinction between these and the player/user actions!
	//!!(One thing's that those tend/should go through the UI, whereas these shouldn't.)
	virtual size_t add_body(Model::World::Body&& obj);
//!!	virtual size_t add_body(Model::World::Body const& obj);
	virtual void   remove_body(size_t ndx);
	size_t add_body(); // add a random one
	void   remove_body(); // delete a random one
	void   add_bodies(size_t n);
	void   remove_bodies(size_t n = -1); // -1 -> all

	unsigned add_player(Model::World::Body&& obj) override;
	void     remove_player(unsigned ndx) override;
	size_t player_entity_ndx([[maybe_unused]] unsigned player_id = 1) const { assert(player_id == 1); return globe_ndx; }
	       Entity& player_entity(unsigned p = 1)       override { assert(entity_count() > player_entity_ndx(p)); return entity(player_entity_ndx(p)); }
	 const Entity& player_entity(unsigned p = 1) const override { assert(entity_count() > player_entity_ndx(p)); return entity(player_entity_ndx(p)); }

	bool poll_and_process_controls() override; // true if there was any input

	bool _ctrl_update_thrusters(); // true if any engine is firing

	// Model event callback implementations... //!!Then move it to some more "modelly place" later, as things will get more complicated.
	void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...) override;
	bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2) override;

	//--------------------------------------------------------------------
	// New overridables introduced:
	virtual void post_zoom_hook([[maybe_unused]] float factor) {}

//----------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	using SimApp::SimApp;
	OON(const OON&) = delete;

//----------------------------------------------------------------------------
// Data / Internals...
//----------------------------------------------------------------------------
protected:
	// See view_control() for these:
	float pan_step_x = 0, pan_step_y = 0;
	float zoom_step = 0;

	size_t globe_ndx = 0;   // Paranoid safety init; see init()!
	size_t clack_sound = 0; // Paranoid safety init; see init()!

	size_t focused_entity_ndx = 0; // The player entity (globe_ndx) by default
};

#endif // __OON__
