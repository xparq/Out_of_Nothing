#ifndef _OSE8975BQ7C785C639406C824X782C6YNB5_
#define _OSE8975BQ7C785C639406C824X782C6YNB5_

#include "OONConfig.hpp"
#include "OONControls.hpp"
#include "OONMainDisplay.hpp"

#include "Engine/SimApp.hpp"

namespace UI { class HUD; } //!!...

#include <cassert>

namespace OON {

class OONMainDisplay;

//============================================================================

//----------------------------------------------------------------------------
class OONApp : public Szim::SimApp
{
//----------------------------------------------------------------------------
// Config/Setup...
//----------------------------------------------------------------------------
public:
	static void show_cmdline_help(const Args& args, const char* banner = nullptr);

protected:
	void init() override;
	void done() override;

	enum HUD_ID { HelpPanel, TimingStats, WorldData, ViewData, ObjectData, Debug };
	virtual UI::HUD& ui_gebi(HUD_ID which) = 0; // get_element_by_id(...)
	bool _show_huds = true;
	void _setup_UI();

//----------------------------------------------------------------------------
// Operations...
//----------------------------------------------------------------------------
public:
	//--------------------------------------------------------------------
	// Player actions...

	void  poll_controls() override;
	bool  perform_control_actions() override; // true if there have been some actions

	// OON gameplay actions...

	virtual void spawn(size_t parent_ndx = 0, size_t n = 1);      //!! requires: 0 == player_entity_ndx()
	void exhaust_burst(size_t entity_ndx = 0, /* Math::Vector2f thrust_vector, */size_t n = 20);
	void chemtrail_burst(size_t emitter_ndx = 0, size_t n = 10);
	void shield_energize(size_t emitter_ndx = 0, /*Math::Vector2f shoot_vector,*/ size_t n = 5);

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
	// View control (note: panning is camera movement in world coordinates)...
	//
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
	enum ViewControlMode : short { UserKeys, UserMouseWheel, AutoFollow };
	bool pan_control(ViewControlMode mode = UserKeys);                              //!!?? should be an override already?
	bool zoom_control(ViewControlMode mode = UserKeys, float mousewheel_delta = 0); //!!?? should be an override already?

	void center_entity(size_t id);
	void center_player(unsigned player_id = 1);
	void follow_entity(size_t id);
	void follow_player(unsigned player_id = 1);

	//!! A more generic _adjust_pan() (or even _adjust_view()) should also exist, e.g.
	//!! to bring back things on-screen, if drifted off!

	//--------------------------------------------------------------------
	// "Meta" controls (not gameplay/player, but admin/user actions)...

	void interact_all(bool state = true)  { world()._interact_all = state; }
	void toggle_interact_all()  { interact_all(!const_world()._interact_all); }

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
	size_t add_random_body_near(size_t base_ndx);
	void   add_random_bodies_near(size_t base_ndx, size_t n);
	void   remove_random_body();
	void   remove_random_bodies(size_t n = -1); // -1 -> all

	unsigned add_player(
		Model::World::Body&& model,
		Szim::Avatar& avatar,
		Szim::VirtualController& controls
	) override;
	void     remove_player(unsigned ndx) override;

protected:
	bool _ctrl_update_thrusters(); // true if any engine is firing

	//----------------------------------------------------------------------------
	struct EmitterConfig
	{
		Math::Vector2f eject_velocity{}; // Relative to the emitter's v
		Math::Vector2f eject_offset{};   // Relative to the emitter's origin
		float v_factor = 0.1f; //!! May be redundant with eject_velocity now!
		float offset_factor = 0.2f;
		float particle_lifetime = Model::World::Body::Unlimited;
		bool  create_mass = true;
		float particle_density = Model::Physics::DENSITY_ROCK * 0.001f;
		Math::Vector2f position_divergence = {5.f, 5.f}; // Scaled by the emitter's radius
		float velocity_divergence = 1.f; //!! Just an exp. "randomness factor" for now!...
		float particle_mass_min{};
		float particle_mass_max{};
		uint32_t color = 0x706080; // 0xRRGGBB
	};
	void _emit_particles(const EmitterConfig& ecfg, size_t emitter_ndx = 0, size_t n = 10,
		Math::Vector2f nozzles[] = nullptr); // 'nozzles' must have n elements if not null,
		                                     // each relative to the origin of the emitter,
		                                     // and normalized to a [-1, 1] bounding box!

	// Model event callback implementations... //!!Then move it to some more "modelly place" later, as things will get more complicated.
	void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...) override;
	bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2) override;


	//------------------------------------------------------------------------
	// Callback impl. (overrides)...
	//------------------------------------------------------------------------
	void updates_for_next_frame() override;
	void time_step(int steps) override;
	void pause_hook(bool newstate) override;
	void onResize(unsigned width, unsigned height) override;
	bool load_snapshot(const char* fname) override; // Needs to reset the rendering cache!
	//bool save_snapshot(const char* fname) override; // Nothing special to do for this.

	//--------------------------------------------------------------------
	// New overridables introduced:
	virtual void resize_shapes(float /*factor*/);
	virtual void resize_shape(size_t /*ndx*/, float /*factor*/);

//----------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	OONApp(int argc, char** argv, OONMainDisplay& main_view);
	OONApp(const OONApp&) = delete;

//----------------------------------------------------------------------------
// Internals...
//----------------------------------------------------------------------------
protected:
	      auto& oon_main_view()         { return (      OONMainDisplay&) main_view(); }
	const auto& oon_main_view()   const { return (const OONMainDisplay&) main_view(); }
	      auto& oon_main_camera()       { return (      OONMainDisplay::MainCameraType&) oon_main_view().camera(); }
	const auto& oon_main_camera() const { return (const OONMainDisplay::MainCameraType&) oon_main_view().camera(); }

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

public://!! Directly accessed by main_view() for now:
	size_t focused_entity_ndx = 0; // The player entity by default
};

} // namespace OON

#endif // _OSE8975BQ7C785C639406C824X782C6YNB5_
