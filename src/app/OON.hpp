#ifndef _OSE8975BQ7C785C639406C824X782C6YNB5_
#define _OSE8975BQ7C785C639406C824X782C6YNB5_

#include "OONConfig.hpp"
#include "OONControls.hpp"
#include "OONMainDisplay.hpp"

#include "Engine/SimApp.hpp"

namespace UI { class HUDStream; }

#include <cassert>

namespace OON {

class OONMainDisplay;

//============================================================================

//----------------------------------------------------------------------------
class OONApp : public Szim::SimApp
//!
//! NOTE: A CRTP impl. would break the compilation barrier between backend-specific
//!	and "pure" code! :-/
//!
//!	template< class AppSysImpl > // CRTP for backend-specifics
//!	class OONApp : public Szim::SimApp
{
friend class Model::World; //!! Later: Not the generic, but the app-specific part only!

//----------------------------------------------------------------------------
// Config/Setup...
//----------------------------------------------------------------------------
public:
	bool show_cmdline_help(const Args& args, const char* banner = nullptr) override; // false: exit afterwards

protected:
	bool init() override;
	bool done() override;
	bool init_cli() override;
	bool done_cli() override;

//----------------------------------------------------------------------------
// Operations...
//----------------------------------------------------------------------------
public:
	//--------------------------------------------------------------------
	void  get_control_inputs() override; // Callback from SimApp

	//!! This might become a std. part of the interface
	bool  react_to_control_inputs() override; //!! NOT ACTUALLY CALLED BY THE ENGINE YET! We just call it ourselves for now...
		// true: actions taken, follow-up needed
	        // false: inputs ignored, follow-up not needed

	// OON gameplay actions...
	virtual void spawn(EntityID parent = 0, unsigned n = 1);      //!! requires: 0 == player_entity_ndx()
	void exhaust_burst(EntityID base_entity = 0, /* Math::Vector2f thrust_vector, */unsigned n = 20);
	void chemtrail_burst(EntityID emitter = 0, unsigned n = 10);
	void shield_energize(EntityID emitter = 0, /*Math::Vector2f shoot_vector,*/ unsigned n = 5);

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

	//
	// View control
	//
	// NOTE: panning is camera movement in screen coordinates, auto-converted to world coords.
	void pan_view(float delta_x, float delta_y);
	void pan_view_x(float delta);
	void pan_view_y(float delta);
	void pan_view_reset();
	void zoom(float factor); // Multiply the current scaling with 'factor'
	bool scroll_locked();    // Auto-panning via pinned focus point or object
	// - Change the zoom ratio by 'amount' (e.g. 5%):
	void zoom_in(float amount);
	void zoom_out(float amount);
	void zoom_reset();

	// - Poll view controls & adjust:
	bool view_control(float mousewheel_delta = 0); //!! override;
		// Returns true if view adjustments have been requested/performed.
		// Note: this is irrespective of modelling, and is also enabled while paused.
	// - view_control() calls these:
	enum ViewControlMode : short { UserKeys, UserMouseWheel, AutoFollow };
	bool pan_control(ViewControlMode mode = UserKeys);                              //!!?? should be an override already?
	bool zoom_control(ViewControlMode mode = UserKeys, float mousewheel_delta = 0); //!!?? should be an override already?

	void center(EntityID entity_id);
	void pan_to_focus(EntityID entity_id);
	void center_player(PlayerID player_id = 1);
//!!	void pan_to_focus(PlayerID player_id = 1);

	//!! A more generic _adjust_pan() (or even _adjust_view()) should also exist, e.g.
	//!! to bring back things on-screen, if drifted off!

	//--------------------------------------------------------------------
	// "Meta" controls (not gameplay/player, but admin/user actions)...

	void interact_all(bool state = true)  { world()._interact_all = state; }
	void toggle_interact_all()  { interact_all(!const_world()._interact_all); }

	void toggle_muting();
	void toggle_music();
	void toggle_sound_fx();

	void toggle_huds();
	bool huds_active();
	void toggle_help();

	//--------------------------------------------------------------------
	// "Internal ops.", rather than "user actions" (well, some still are, e.g. for testing)
	//!!Make a proper distinction between these and player/user actions!
	//!!(One thing's that those tend/should go through the UI, whereas these shouldn't.)
	EntityID add_random_body_near(EntityID base_entity);
	void     add_random_bodies_near(EntityID base_entity, size_t n);
	void     remove_random_body();
	void     remove_random_bodies(size_t n = -1); // -1 -> all

	unsigned add_player(
		Entity&& model,
		Szim::Avatar& avatar,
		Szim::VirtualController& controls
	) override;
	void     remove_player(PlayerID ndx) override;

protected:
	bool _ctrl_update_thrusters(); // true if any engine is firing

	//------------------------------------------------------------------------
	// Op. implementations/overrides...
	void updates_for_next_frame() override;
	EntityID add_entity(Entity&& temp) override;
	void remove_entity(EntityID ndx) override;
//	void transform_entity(EntityTransform f) override;
//	void transform_entity(EntityTransform_ByIndex f) override;
	//--------------------------------------------------------------------
	void resize_shape(size_t /*ndx*/, float /*factor*/) override;
	void resize_shapes(float /*factor*/) override;

	void time_step(int steps) override;
	bool load_snapshot(const char* fname) override; // Needs to reset the rendering cache!
	//bool save_snapshot(const char* fname) override; // Nothing special to do for this one.

	//------------------------------------------------------------------------
	// Model callback implementations...
	//!! Move these out of the direct app code to an app-level custom model class set!
	//!! And then the model callback mechanism could be simplified to not doing it in
	//!! the core abstract Model at all, but in the custom layer, only when needed.
	void init_world_hook() override;
	void undirected_interaction_hook(Model::World* w, Entity* obj1, Entity* obj2, float dt, double distance, ...) override;
	void directed_interaction_hook(Model::World* w, Entity* source, Entity* target, float dt, double distance, ...) override;
	bool touch_hook(Model::World* w, Entity* obj1, Entity* obj2) override;

	//------------------------------------------------------------------------
	// Other callback impl. (overrides)...
	void pause_hook(bool newstate) override;
	void onResize(unsigned width, unsigned height) override;

//----------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	OONApp(const Szim::RuntimeContext& runtime, int argc, char** argv, OONMainDisplay& main_view);
	OONApp(const OONApp&) = delete;

//----------------------------------------------------------------------------
// Internals...
//----------------------------------------------------------------------------
protected:
	      auto& oon_main_view()         { return (      OONMainDisplay&) main_view(); }
	const auto& oon_main_view()   const { return (const OONMainDisplay&) main_view(); }
	      auto& oon_main_camera()       { return (      OONMainDisplay::MainCameraType&) oon_main_view().camera(); }
	const auto& oon_main_camera() const { return (const OONMainDisplay::MainCameraType&) oon_main_view().camera(); }

	// UI helpers...
	//!! RECONCILE WITH THE UI namespace!... Somehow...
	//!!struct _UI_ {
		enum HUD_ID { HelpPanel, TimingStats, WorldData, ViewData, ObjMonitor, Debug };
		bool _ui_show_huds = true;

		void ui_setup(/*!! ??const?? OONApp& app!!*/); //!! See also _sync_to_model_state!
		// Update the UI after bulk model changes:
		//!!void _sync_to_model_state(const SimApp& app/*!!, ...what (optionally, for optimization)...!!*/);

		void ui_setup_HUDs();
		void ui_setup_HUD_Help();
		void ui_setup_HUD_World();
		void ui_setup_HUD_Time();
		void ui_setup_HUD_View();
		void ui_setup_HUD_ObjMonitor();
	//!!} ui;
	//!!using HUD_ID = _UI_::HUD_ID; using enum _UI_::HUD_ID; // Also import all the values!
	virtual UI::HUDStream& ui_gebi(HUD_ID which) = 0; // get_element_by_id(...)

	// Chores after loading a new model world:
	void _on_snapshot_loaded(); // Updates the UI etc.

//----------------------------------------------------------------------------
// Internals - Data...
//----------------------------------------------------------------------------
protected:
	OONConfig appcfg; // See also syscfg from this->SimApp

	OONController controls;

	bool  chemtrail_releasing = false;
	short chemtrail_fx_channel = Szim::Audio::INVALID_SOUND_CHANNEL;

	short shield_fx_channel = Szim::Audio::INVALID_SOUND_CHANNEL;
	int   shield_active = 0; // 1: active; <0: depleted, recovering
	float shield_depletion_timestamp; // session time in s

	// See view_control() for these:
	float _pan_step_x = 0, _pan_step_y = 0;
	float _zoom_step = 0;

	// Built-in player avatars (see init()):
	size_t tx_jesus;
	size_t tx_santa;
	size_t tx_kittygod;

	// See init()!
	//!! Use sz::lockers for the storage:
	size_t snd_clack;
	size_t snd_plop1;
	size_t snd_plop2;
	size_t snd_plop3;
	size_t snd_pwhiz;
	size_t snd_jingle_loop;
	size_t snd_shield;

public://!! Directly accessed by e.g. main_view() and the ObjMonitor HUD...:
	EntityID focused_entity_ndx = 0; // Player #1 by default //!!CRYPTIC HARDCODING
	EntityID hovered_entity_ndx = Entity::NONE;
};

} // namespace OON

#endif // _OSE8975BQ7C785C639406C824X782C6YNB5_
