// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "OON.hpp"

//!! Should be internal to the Lorem-Ipsum Drive thruster, but for now...:
#include "Model/Emitter/SkyPrint.hpp"

#include "Engine/Backend/HCI.hpp"
#include "sfw/GUI.hpp" //!! Used to be in OONApp only, but since scroll_locked() requires it...
                       //!! (And sooner or later it must be usable unrestricted anyway!
#include "UI/hud.hpp"  //!! <-- And also this would be integrated there, too, eventually.
                       //!! And we're already using keystate() here, too, shamelessly! ;) )

#include "sz/sign.hh"

#include <cstdlib>
	using std::rand; // and the RAND_MAX macro!
#include <cmath>
	using std::pow;
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>
#include "sz/debug.hh"

using namespace Szim;
using namespace Model;
using namespace Math;
using namespace UI;
using namespace std;


namespace OON {

//----------------------------------------------------------------------------
OONApp::OONApp(int argc, char** argv, OONMainDisplay& main_view)
	: SimApp(argc, argv, main_view)
	, appcfg(cfg, args) //!! appcfg(SimApp::syscfg)
{
//!! This shouldn't be needed, the engine should take care of it: #462!
//!! And the view resize should also implicitly take care of any camera adjustments, too, so
//!! this commented part would be the one that's actually needed, and the cam. stuff deleted below!
//!!	oon_main_view().resize((float)backend.hci.window().width,
//!!	                         (float)backend.hci.window().height);
	oon_main_camera().resize((float)main_window_width(),
	                         (float)main_window_height());
}

//----------------------------------------------------------------------------
void OONApp::init() //override
/*!
    The data watcher HUD setup is tied directly to the model world state (incl. e.g. the
    player objects), so model init (init_world) MUST happen before UI init (ui_setup)!
!*/
{_
ZoneScoped;

	// Images...
	//!!
	//!! MUST COME BEFORE CALLING add_entity() or its friends! :-o
	//!!
	avatars.emplace_back(Avatar{ .image_path = "image/HoryJesus.jpg", .tint_RGBA = 0xffff99ff });
	tx_jesus = avatars.size() - 1;
	avatars.emplace_back(Avatar{ .image_path = "image/SantaMatt.jpg", .tint_RGBA = 0xccccffff });
	tx_santa = avatars.size() - 1;
	avatars.emplace_back(Avatar{ .image_path = "image/KittyGod.jpg"});
	tx_kittygod = avatars.size() - 1;

	//!! OMG, also the cringefest!... The view.reset() below is needed to sync the view._avatars list with the "real" avatars,
	//!! which is -- alas, counterintuitively! -- expected by (something in) add_entity(), I guess! :-o :-/
	//!! -> WHICH SHOULD BE FIXED, MOST LIKELY!
cerr << "DBG> Right after loading the avatar images (in "<<__FUNCTION__<<"):\n";
	oon_main_view().reset(); //!!REPLACE THIS WITH A SANER WAY TO SYNC THE VIEW TO THE APP STATE (i.e. the avatars)!

	//!!
	//!! MOVE THIS TO SimApp! But can't yet be, as the stupid avatar loading must happen first! :-o
	//!! (Which should be either fully decoupled from the model (so the order shouldn't matter),
	//!! of integrated right into it (so it could stop being a mere gimmick)!)
	//!!
	//!! SHOULD BE CALLED BY the default impl. of SimApp::init(), AT THE APPROPRIATE TIME
	//!! (E.G. IN ACCORDANCE WITH SESSION LOADING!), BUT FOR NOW, AS THAT POINT DOESN'T EXIST YET...:
	//!!
	init_world(); //!! Also, it can only be done after getting rid of the RAII app init (#483) to regain virtual dispatch...

	// Note also that init_world() is eventually wasted if we're also loading a session,
	// but that's a price paid for *some* level of simplicity in the init sequence.
	//!!
	//!! BTW: Even the player entity(/-ies) get(s) deleted/recreated by a load, which should change in the future!
	//!! (Unless it would be made reasonable to reload a "multiplayer session" -- whatever that (and saving it) should even mean...)
	//!!

	//!!
	//!! MOVE THE SESSION LOGIC TO SimApp!
	//!! (NOTE: Can't move it to init_world_hook() either, and that may not even exist!...)
	//!!
	// Restore or start session (currently just loading a snapshot...) if requested (i.e. --session=name)...
	if (args["session"]) { // If empty, a new unnamed session will be started; see also at done()!
		session.open(args("session")); // So, a previously autosaved unnamed session state will NOT be loaded implicitly!
	} else {
		//!! Here should be a "load default initial world state" thing...
		//!!
		//!! session.create(); // call back to the app to build default state
		//!!
		//!! Or even just this would be fine:
		//!! load_snapshot(DEFAULT);
		//!!
		//!! - But preferably also being able to load from some text format
		//!!   (TOML etc.), to finally replace this sad little hardcoding here:
		//!!
	}

	// App-level cmdline options (overrides)...
	// Note: the (!!actually: "some"...!!) system-/engine-level options have been processed/applied already!
	//!! RECONCILE THIS WITH THE WORLD STATE ATTRIBUTES IN world_init_hook()!
	try { // <- Absolutely required, as sto...() are very throw-happy.
		if (!args("zoom-adjust").empty()) {
			// Trim the original base scale level:
			if (float trim = stof(args("zoom-adjust")); trim) {
				oon_main_camera().cfg.base_scale *= trim;
				zoom_reset();
			}
		}
	} catch(...) {
		cerr << __FUNCTION__ << ": ERROR processing/applying some cmdline args!\n";
		request_exit(-1);
		return;
	}

	// Focus on Player #1:
	focused_entity_ndx = player_entity_ndx(1); //!!... See init_world_hook()!

	//!! Absolutlely MUST come after the world init (i.e. session loading!)
	//!! Also: the widgets are only (or mostly) initialized from prior app state, and not updated by (most)
	//!! app-level ops, so with an early UI setup some widgets may get out of sync by later app changes!
	ui_setup(); //!!?? Should this come even after the main_view init? (Or will the main view eventually need to depend on the UI??)

	//!! Also this, called manually... Sigh... A temp. workaround for #472:
	//!! (And this is a pretty arbitrary place for that, too! :-o :-/ )
	//!! ALSO: if there was a session load, it has already called it, so there
	//!! are double debug outputs for it...
cerr << "DBG> After UI setup (in "<<__FUNCTION__<<"):\n";
	oon_main_view().reset();

	// Audio...
	//!
	//!! Later, with more mature session mgmt., music loading etc. should NOT
	//!! happen here, ignoring non-app-level user preferences etc...
	//!
	// Note: muting by --snd=off has been taken care of by the engine already,
	// so we can just go ahead and play things nonchalantly. ;)
	snd_clack       = backend.audio.add_sound(string(cfg.asset_dir + "sound/clack.wav").c_str());
	snd_plop1       = backend.audio.add_sound(string(cfg.asset_dir + "sound/plop1.wav").c_str());
	snd_plop2       = backend.audio.add_sound(string(cfg.asset_dir + "sound/plop_low.flac").c_str());
	snd_plop3       = backend.audio.add_sound(string(cfg.asset_dir + "sound/reverbed_plop.ogg").c_str());
	snd_pwhiz       = backend.audio.add_sound(string(cfg.asset_dir + "sound/pwhiz.wav").c_str());
	snd_jingle_loop = backend.audio.add_sound(string(cfg.asset_dir + "sound/jingle_discharge.ogg").c_str());
	snd_shield	= backend.audio.add_sound(string(cfg.asset_dir + "sound/shield1.flac").c_str());

	backend.audio.play_music(cfg.background_music.c_str());
	//backend.audio.play_music(sz::prefix_if_rel(asset_dir, "music/extra sonic layer.ogg"));
	//backend.audio.play_sound(snd_plop_low, true); //!! just checking

//!!IPROF_SYNC_THREAD;
} // init


//----------------------------------------------------------------------------
void OONApp::done() //override
{
//	cerr << __FUNCTION__ << ": Put any 'onExit' tasks (like saving the last state) here!...\n";

	//!! MOVE THE SESSION LOGIC TO SimApp:
	// Let the session-manager auto-save the current session (unless disabled with --session-no-autosave; see SimApp::init()!)
	if (args["session"]) { // If empty and no --session-save-as, it will be saved as "UNNAMED.autosave" or sg. like that.
		session.close();
	}
}

void OONApp::init_world_hook() //override
{_
	auto& w = world(); //!! Using the default-constr'd (so basically undefined!)
	                   //!! model world implicitly created by SimApp!... :-o

	// Add the "Player Superglobe" first
	//!!
	//!! THIS MUST COME BEFORE CALLING add_random_bodies_near(player)! :-o
	//!!
cerr << "DBG> Adding player #1...\n";
	auto player_id [[maybe_unused]] = //! Only for assertions in a DEBUG build!
	add_player(
		{//.r = w.CFG_GLOBE_RADIUS, // Redundant: will be calculated!
		 .density = appcfg.get("sim/player_globe_density", Phys::DENSITY_OF_EARTH / 10.f),
		 .p = {0,0}, .v = {0,0},
		 .color = 0xffff20,
		 .mass = appcfg.get("sim/player_globe_mass", 50 * Phys::MASS_OF_EARTH)},
		avatars[tx_jesus],
		controls
	);
	assert(players.size() == 1);
	assert(entity_count() > player(player_id).entity_ndx);
	assert(player_entity_ndx() == player(player_id).entity_ndx);


	try { // <- Absolutely required, as sto...() are very throw-happy.
		// Doing the ones that can't fail first, so an excpt. won't skip them:
		if (appcfg.get("sim/global_interactions", cfg.global_interactions)) { //!! :-/ EHH, RESOLVE THIS compulsory defult misery!
			interact_all();
		}; if (args["bodies"]) {
			auto n = stoi(args("bodies"));
			add_random_bodies_near(player_entity_ndx(), n < 0 ? 0 : n); //! Dodge a possible overflow of n
		   } else if (!args["session"]) { //! Only if no session being loaded...
		                                //!! MAKE THIS CHECK (FOR A SESSION) MUCH MORE ROBUST!!!
cerr << "DBG> Creating two small moons by default...\n";
			// Add 2 "moons" with fixed parameters (mainly for testing):
			add_entity({//.r = w.CFG_GLOBE_RADIUS/10, // Redundant: will be calculated!
			            .p = {w.CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -w.CFG_GLOBE_RADIUS * 2},
			            .color = 0xff2020, .mass = 3e24f});
			add_entity({//.r = w.CFG_GLOBE_RADIUS/7, // Redundant: will be calculated!
			            .p = {-w.CFG_GLOBE_RADIUS * 1.6f, +w.CFG_GLOBE_RADIUS * 1.2f}, .v = {-w.CFG_GLOBE_RADIUS*1.8, -w.CFG_GLOBE_RADIUS*1.5},
			            .color = 0x3060ff, .mass = 3e24f});
		}; if (args["friction"]) {
			float f = stof(args("friction"));
			world().friction = f;
		};
	} catch(...) {
		cerr << __FUNCTION__ << ": ERROR processing/applying some cmdline args!\n";
		request_exit(-1);
		return;
	}
}


//----------------------------------------------------------------------------
// (Unrelated to onResize(), but... where else would be better to put these...)
void OONApp::resize_shapes(float factor) //override
{
	oon_main_view().resize_objects(factor);
}

void OONApp::resize_shape(size_t ndx, float factor) //override
{
	oon_main_view().resize_object(ndx, factor);
}


//----------------------------------------------------------------------------
unsigned OONApp::add_player(World::Body&& obj, Avatar& avatar, VirtualController& ctrlr) //override
{
	// These are the player modelling differences:
	obj.add_thrusters();
	obj.superpower.gravity_immunity = appcfg.get("sim/player_antigravity", true);
	obj.superpower.free_color = true;
	obj/*.superpower*/.lifetime = Entity::Unlimited; //!!?? Should this be a superpower instead?

	auto p_ent = (unsigned) //!! Blatant narrowing conv., hoping entity_count() will never overflow `unsigned`...
		add_entity(std::forward<World::Body>(obj));

	players.emplace_back(p_ent, avatar, ctrlr);
	assert(players.size());
	return (unsigned)players.size(); //! NOT size()-1!...
}

void OONApp::remove_player(unsigned)
//!! Just deleting it would shift the array and invalidate all the subsequent indexes! :-/
//!! A simple (not std!) player index map is needed! Or is this just the right job for sz::lockers?! :-o
//!! However, that's a fixed number of players.
//!! Must distinguish between a local game with *very few* local players,
//!! and servers with a huge number of players!
{
}


//----------------------------------------------------------------------------
void OONApp::poll_controls() //override
{
	controls.update(); // Refreshing polled states now, nothing else
		//!! Should be moved to the event loop, but only after the
		//!! current hamfisted threading is resolved; there could be
		//!! much delay between polling and actually reacting now!
}


//----------------------------------------------------------------------------
bool OONApp::perform_control_actions() //override
{
	bool action = false;

	// Thruster plumes
	if (_ctrl_update_thrusters()) {
		action = true;
		exhaust_burst(player_entity_ndx(), 5); // 5 per thruster by default, unless reconfigured
	}

	// "Chemtrail" release
	if (controls.Chemtrail) {
		action = true;
		chemtrail_burst(player_entity_ndx(), appcfg.chemtrail_burst_particles);
		if (!chemtrail_releasing) { // Just starting...
			chemtrail_releasing = true;
			chemtrail_fx_channel = backend.audio.play_sound(snd_jingle_loop, {.loop=true});
				//! Can be INVALID_SOUND_CHANNEL, if not playing actually (disabled, muted etc.)
		}
	} else if (chemtrail_releasing) { // Stop it!
		chemtrail_releasing = false;
		backend.audio.kill_sound(chemtrail_fx_channel); // Tolerates INVALID_SOUND_CHANNEL.
		chemtrail_fx_channel = Audio::INVALID_SOUND_CHANNEL;
	}


	// Shield
	if (shield_active < 0) { // Disabled while recovering...
		++shield_active;
	} else {
		if (controls.Shield) {
			action = true;
			if (shield_active == 0) { // Just starting?
				shield_active = 1;
				shield_depletion_timestamp = time.real_session_time + appcfg.shield_depletion_time;
				shield_fx_channel = backend.audio.play_sound(snd_shield, {.priority=1});
					//! Can be INVALID_SOUND_CHANNEL, if not playing actually (disabled, muted etc.)
			}
			// Feed the shield:
			shield_energize(player_entity_ndx(), appcfg.shield_burst_particles); // "replenish rate" -- effectively density!
		}

		if (shield_active > 0) {
			// Depleted?
			if (time.real_session_time > shield_depletion_timestamp) {

				shield_active = -int(appcfg.shield_recharge_time / float(avg_frame_delay));
cerr << "- Shield depleted! Recharging for " << -shield_active << " frames...\n";

				//!! Not killing the sound, as its length is supposed to be the same
				//!! as the shield depletion time, and an accidental miscalibration
				//!! error could at least be noticed this way! :)
				//!!backend.audio.kill_sound(shield_fx_channel); // Tolerates INVALID_SOUND_CHANNEL
			}
			// Stopped firing?
			else if (!controls.Shield) {
				shield_active = 0;
				backend.audio.kill_sound(shield_fx_channel); // Tolerates INVALID_SOUND_CHANNEL
			}
		}
	}

	//!! No concept of multiple players whatsoever yet:
	if (action) player_mark_active();

	return action;
}

//----------------------------------------------------------------------------
void OONApp::up_thruster_start()    { entity(player_entity_ndx()).thrust_up.thrust_level(appcfg.player_thrust_force); }
void OONApp::down_thruster_start()  { entity(player_entity_ndx()).thrust_down.thrust_level(appcfg.player_thrust_force); }
void OONApp::left_thruster_start()  { entity(player_entity_ndx()).thrust_left.thrust_level(appcfg.player_thrust_force); }
void OONApp::right_thruster_start() { entity(player_entity_ndx()).thrust_right.thrust_level(appcfg.player_thrust_force); }
void OONApp::up_thruster_stop()     { entity(player_entity_ndx()).thrust_up.thrust_level(0); }
void OONApp::down_thruster_stop()   { entity(player_entity_ndx()).thrust_down.thrust_level(0); }
void OONApp::left_thruster_stop()   { entity(player_entity_ndx()).thrust_left.thrust_level(0); }
void OONApp::right_thruster_stop()  { entity(player_entity_ndx()).thrust_right.thrust_level(0); }

bool OONApp::_ctrl_update_thrusters()
{
	auto drv = false;
	if (controls.ThrustUp)    { drv = true;    up_thruster_start(); } else    up_thruster_stop();
	if (controls.ThrustDown)  { drv = true;  down_thruster_start(); } else  down_thruster_stop();
	if (controls.ThrustLeft)  { drv = true;  left_thruster_start(); } else  left_thruster_stop();
	if (controls.ThrustRight) { drv = true; right_thruster_start(); } else right_thruster_stop();
	return drv;
}


//----------------------------------------------------------------------------
void OONApp::pan_reset()
{
	_pan_step_x = _pan_step_y = 0;

	oon_main_camera().center_to({0, 0});

	// Since the player entity may have moved out of view, stop focusing on it:
	//!!
	//!!?? What is the rule for Scroll Lock in this case?
	//!!The key should be turned off!...
	//!!
	focused_entity_ndx = ~0u; //!!... Whoa! :-o See updates_for_next_frame()!
}

void OONApp::pan(Vector2f delta) { oon_main_camera().pan_x(delta.x); oon_main_camera().pan_y(delta.y); }
void OONApp::pan_x(float delta)  { oon_main_camera().pan_x(delta); }
void OONApp::pan_y(float delta)  { oon_main_camera().pan_y(delta); }

void OONApp::pan_to_center(size_t entity_id)
{
	oon_main_camera().center_to(Vector2f(entity(entity_id).p));
	oon_main_camera().focus_to({0, 0});
//!!??	oon_main_camera().focus_to(entity(id).p);
}

void OONApp::center_player(unsigned player_id)
{
	pan_to_center(player_entity_ndx(player_id));
}

void OONApp::pan_to_focus(size_t id)
{
	auto vpos = oon_main_camera().world_to_view_coord(Vector2f(entity(id).p));
	oon_main_camera().pan(vpos - oon_main_camera().focus_offset);
}

/*!! OBSOLETE:
void OONApp::follow_player(unsigned player_id)
{
	pan_to_focus(player_entity_ndx(player_id));
}
!!*/


void OONApp::zoom_reset()
{
	//! Can't just call oon_main_camera().reset_zoom(): we need to adjust the cached shapes, too!

	auto old_scale = oon_main_camera().scale(); // See below why this is needed...

	oon_main_camera().reset_zoom();

	//!! Alas, no direct (abs.) (re)set_scale method for this:
	resize_shapes(oon_main_camera().scale()/old_scale); // Adjust scaling (relative to current level)
	//!! (Also can't just trivially call zoom(...) here either, as the two zoom levels would go out of sync.)
}

void OONApp::zoom(float factor)
{
	//!!?? if (!factor) factor = 1.0f; //! 0 makes 0 sense, so...
	oon_main_camera().zoom(factor);
	resize_shapes(factor); // Adjust scaling (relative to current level)
	//!!scale_shapes(level); // Set scaling (abs. level)
}
// These can't call oon_main_camera().zoom_in/out directly either, because we need to trigger our zoom_hook!...
void OONApp::zoom_in (float amount) { zoom(1.f + amount); }
void OONApp::zoom_out(float amount) { zoom(1.f / (1.f + amount)); }


/*!!
void OONApp::zoom(float factor)
{
//auto viewpos = oon_main_camera().world_to_view_coord(player_entity().p);
//cerr << "- focus vs player diff: " << (viewpos - oon_main_camera().focus_offset).x << ", " << (viewpos - oon_main_camera().focus_offset).y << '\n';

//!!pre_zoom_hook(factor);
	// Compensate for zoom displacement when the player object is not centered
	auto v = oon_main_camera().world_to_view_coord(oon_main_camera().offset);
	pan((oon_main_camera().focus_offset - v) * oon_main_camera().zoom/factor);
//	auto viewpos = oon_main_camera().focus_offset + oon_main_camera().offset;
//	pan(viewpos - viewpos/factor);

//	auto vpos = oon_main_camera().world_to_view_coord(oon_main_camera().offset);
//	pan(oon_main_camera().focus_offset/factor);

	oon_main_camera().zoom *= factor;

	resize_shapes(factor);
}

//----------------------------------------------------------------------------
void OONApp::_adjust_pan_after_zoom(float factor)
{
	// If the new zoom level would put the player object out of view, reposition the view so that
	// it would keep being visible; also roughly at the same view-offset as before!

	auto visible_R = player_entity().r * oon_main_camera().zoom; //!! Not a terribly robust method to get that size...

	if (abs(vpos.x) > cfg.VIEWPORT_WIDTH/2  - visible_R ||
	    abs(vpos.y) > cfg.VIEWPORT_HEIGHT/2 - visible_R)
	{
cerr << "R-viewsize: " << oon_main_camera().zoom * plm->r
	 << " abs(vpos.x): " << abs(vpos.x) << ", "
     << " abs(vpos.u): " << abs(vpos.y) << endl;

		pan_to_player(offset);
		pan_to_entity(player_entity_ndx(), vpos * CFG_ZOOM_CHANGE_RATIO); // keep the on-screen pos!
//		zoom_out(); //!! Shouldn't be an infinite zoom loop (even if moving way too fast, I think)
	}
}
!!*/

bool OONApp::scroll_locked()
{
	return controls.PanLock || controls.PanFollow
		|| sfw::get<sfw::CheckBox>("  - forced follow", false);
		//!!?? Should this GUI poll actually be in controller.update()?!...
		//!!?? Kinda depends on the intent of that UI element: input emu., or
		//!!?? "high-level control" <-- but then this should make some actual sense! :)
}


//----------------------------------------------------------------------------
bool OONApp::view_control(float mousewheel_delta) //!!override
{
	auto action = false;
	action |= pan_control();
	action |= zoom_control(mousewheel_delta ? UserMouseWheel : UserKeys, mousewheel_delta);
	return action;
}

//----------------------------------------------------------------------------
bool OONApp::pan_control([[maybe_unused]] ViewControlMode mode) //!!override
//!! ViewControlMode support is not implemented here at all; see zoom_control()!
{
	//
	// Calibrated for 30FPS; normalized below (#306)
	//
	// (In theory, it could've been normalized by adjusting the sampling rate, rather than
	// each of the values below, but I suspect that the uneven reactions could be noticeable.
	// Also, since I've set things for 30 PFS, skip-frame compensation can't help with <30 FPS!...)
	//
	//AUTO_CONST CFG_PAN_INITIAL_STEP = 5; // pixel
	AUTO_CONST CFG_PAN_EASEOUT_STEP = 1; // +/- pixel

	auto fps_factor = (float)avg_frame_delay * 30.f; // Adjust relative to the 30 FPS calibration reference
		//! Note: this weird cast is required to avoid an "operator ambiguous" error!

	//auto CFG_PAN_INITIAL_STEP_fps = CFG_PAN_INITIAL_STEP * fps_factor;
	auto CFG_PAN_EASEOUT_STEP_fps = CFG_PAN_EASEOUT_STEP * fps_factor;
//!!??auto CFG_PAN_EASEOUT_STEP_fps = CFG_PAN_EASEOUT_STEP;

	auto action = false;
	if (controls.PanUp)    { action = true; _pan_step_y += CFG_PAN_EASEOUT_STEP_fps; } // = !_pan_step_y ? -CFG_PAN_INITIAL_STEP_fps : _pan_step_y - CFG_PAN_EASEOUT_STEP_fps; } // approach 2
	if (controls.PanDown)  { action = true; _pan_step_y -= CFG_PAN_EASEOUT_STEP_fps; } // = !_pan_step_y ?  CFG_PAN_INITIAL_STEP_fps : _pan_step_y + CFG_PAN_EASEOUT_STEP_fps; }
	if (controls.PanLeft)  { action = true; _pan_step_x -= CFG_PAN_EASEOUT_STEP_fps; } // = -CFG_PAN_INITIAL_STEP_fps; } // approach 1
	if (controls.PanRight) { action = true; _pan_step_x += CFG_PAN_EASEOUT_STEP_fps; } // =  CFG_PAN_INITIAL_STEP_fps; }

	// Ease-out:
	if (!action) {
		if (_pan_step_x) _pan_step_x -= sz::sign(_pan_step_x) * CFG_PAN_EASEOUT_STEP_fps;
		if (_pan_step_y) _pan_step_y -= sz::sign(_pan_step_y) * CFG_PAN_EASEOUT_STEP_fps;
		if (abs(_pan_step_x) < CFG_PAN_EASEOUT_STEP_fps) _pan_step_x = 0;
		if (abs(_pan_step_y) < CFG_PAN_EASEOUT_STEP_fps) _pan_step_y = 0;
	}

	// Do the actual panning:
	if (scroll_locked()) { // Allow adjusting the pan position while follow-locked (with Shift, Scroll Lock etc.)
		if (_pan_step_x) oon_main_camera().move_focus_x(- _pan_step_x * fps_factor);
		if (_pan_step_y) oon_main_camera().move_focus_y(- _pan_step_y * fps_factor);
	} else {
		if (_pan_step_x) pan_x(_pan_step_x * fps_factor);
		if (_pan_step_y) pan_y(_pan_step_y * fps_factor);
	}

	return action;
}

//----------------------------------------------------------------------------
bool OONApp::zoom_control([[maybe_unused]] ViewControlMode mode, float mousewheel_delta) //!!override
//!! ViewControlMode is not actually used, as autofollow just emulates a mousewheel action directly! :-o
{
	// Note: the mouse-wheel case needs no calibration, as it's triggered
	// directly by the mouse events, independently of frame rate!
	// See more about FPS norm. at pan_control()!
	static const float CFG_ZOOM_BUTTONS_CHANGE_RATE = appcfg.get("controls/zoom_button_strength", 0.08f); // change / Δt
	static const float CFG_ZOOM_MOUSEWHEEL_FACTOR   = appcfg.get("controls/zoom_wheel_strength", 0.12f);  // change / event
	static const float CFG_ZOOM_EASEOUT_STEP        = appcfg.get("controls/zoom_inertia", 0.1f)           // change / Δt
	                                                  * CFG_ZOOM_BUTTONS_CHANGE_RATE;                     // ...normalized to chg. rate

	auto fps_factor = (float)avg_frame_delay * 30.f; // Adjust relative to the 30 FPS calibration reference
		//! Note: this weird cast is required to avoid an "operator ambiguous" error (on avg_frame_delay)!

	auto CFG_ZOOM_BUTTONS_CHANGE_RATE_fps = CFG_ZOOM_BUTTONS_CHANGE_RATE * fps_factor;
	auto CFG_ZOOM_EASEOUT_STEP_fps = CFG_ZOOM_EASEOUT_STEP * fps_factor;

	auto action = false;
	// Mouse-wheel zoom?
	if (mousewheel_delta) { action = true; _zoom_step = mousewheel_delta * CFG_ZOOM_MOUSEWHEEL_FACTOR; }
	// Keyboard zoom?
	//!! TRY WITHOUT THAT `else` below, so that power users can mega-zoon using both! :)
	else if (/*mode == UserKeys &&*/controls.ZoomIn) { action = true; _zoom_step += _zoom_step == 0 ? // Speed up gradually...
	                                                        CFG_ZOOM_BUTTONS_CHANGE_RATE_fps : CFG_ZOOM_EASEOUT_STEP_fps; }
	else if (/*mode == UserKeys &&*/controls.ZoomOut){ action = true; _zoom_step -= _zoom_step == 0 ? // ...by the easeout step!
	                                                        CFG_ZOOM_BUTTONS_CHANGE_RATE_fps : CFG_ZOOM_EASEOUT_STEP_fps; }
	// Ease-out:
	if (!action) {
		if (_zoom_step) _zoom_step -= sz::sign(_zoom_step) * CFG_ZOOM_EASEOUT_STEP_fps;
		if (abs(_zoom_step) < CFG_ZOOM_EASEOUT_STEP_fps) _zoom_step = 0;
	}

	// Do the actual zooming:
	if (_zoom_step) {
		if (_zoom_step > 0) zoom_in(  _zoom_step * fps_factor);
		else                zoom_out(-_zoom_step * fps_factor);
	}

	return action;
}


//----------------------------------------------------------------------------
//!!Move chores like this to the Szim API!
void OONApp::toggle_muting() { backend.audio.toggle_audio(); }
void OONApp::toggle_music() { backend.audio.toggle_music(); }
void OONApp::toggle_sound_fx() { backend.audio.toggle_sounds(); }


//----------------------------------------------------------------------------
void OONApp::undirected_interaction_hook(Model::World* w, Entity* obj1, Entity* obj2, float dt, double distance, ...) //override
{w, obj1, obj2, dt, distance;
}

void OONApp::directed_interaction_hook(Model::World* w, Entity* source, Entity* target, float dt, double distance, ...) //override
{w, source, target, dt, distance;
//	if (!obj1->is_player())
//		obj1->color += 0x3363c3;

/*!!
	auto dx = source->p.x - target->p.x,
	     dy = source->p.y - target->p.y;
//	auto distance = Math::mag2(dx, dy);
	float G_per_DD = w->gravity / (distance * distance);

	float a_target = G_per_DD * source->mass;
	Vector2f dv_target = Vector2f{dx * a_target, dy * a_target} * dt; //!! Fake "vectorization"!...
	//!! Or perhaps: Vector2f dv_target(dx / distance * g, dy / distance * g) * dt;
	target->v += dv_target;

//!!/*!! Alternatively, this would normally be done in its own separate loop cycle, but that's 10-12% SLOWER! :-o
	float a_source = -G_per_DD * target->mass;
	Vector2f dv_source = Vector2f{dx * a_source, dy * a_source} * dt; //!! Fake "vectorization"!...
	//!! Or perhaps: Vector2f dv_source(dx / distance * g, dy / distance * g) * dt;
	source->v += dv_source;
!!*/
}

//----------------------------------------------------------------------------
bool OONApp::touch_hook(World* w, World::Body* obj1, World::Body* obj2)
{w;
	if (obj1->is_player() || obj2->is_player()) {
		backend.audio.play_sound(snd_clack);
	}

	obj1->T += 100;
	obj2->T += 100;

	obj1->recalc();
	obj2->recalc();

	return false; //!!Not yet used!
}



//----------------------------------------------------------------------------
void OONApp::add_random_bodies_near(size_t base_ndx, size_t n)
{
	while (n--) add_random_body_near(base_ndx);
}

//----------------------------------------------------------------------------
void OONApp::remove_random_bodies(size_t n/* = -1*/)
{
	if (!n) return;

	backend.audio.play_sound(snd_pwhiz);

	if (n == (unsigned)-1) n = entity_count();
	while (n--) remove_random_body();
}


//----------------------------------------------------------------------------
size_t OONApp::add_entity(Entity&& temp) //override
// Add new entity (moved) from a temporary template obj.
{
	auto ndx = SimApp::add_entity(temp);
	// Pre-cache shapes for rendering... (!! May be pointless, but this is just what I started with...)
	oon_main_view().create_cached_shape(entity(ndx), ndx); // temp is dead now... ()
	return ndx;
}

//----------------------------------------------------------------------------
size_t OONApp::add_random_body_near(size_t base_ndx)
//!! This is still a version of (mass-ignoring) spawn()!...
//!! Callers may not know, but this depends on the properties of the player body!
//!! See also spawn() (that calls this), which is at least is explicit about it!
{
	//!! These should be either static, or actually depend on dynamic state...
	const auto& cw = const_world();
//	auto constexpr r_min = cw.CFG_GLOBE_RADIUS / 9;
//	auto constexpr r_max = cw.CFG_GLOBE_RADIUS * 3;
	auto constexpr p_range = cw.CFG_GLOBE_RADIUS * 30;
	auto constexpr v_range = cw.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!

	const auto& base = const_entity(base_ndx);
	auto M_min = base.mass / 9;
	auto M_max = base.mass * 3;

//cerr << "Adding new object #" << cw.bodies.size() + 1 << "...\n";
	return add_entity({
		.p = { (rand() * p_range) / RAND_MAX - p_range/2 + base.p.x,
		       (rand() * p_range) / RAND_MAX - p_range/2 + base.p.y },
		.v = { (rand() * v_range) / RAND_MAX - v_range/2 + base.v.x * 0.05f,
		       (rand() * v_range) / RAND_MAX - v_range/2 + base.v.y * 0.05f },
		.color = 0xffffff & ((uint32_t) rand() * rand()),
		.mass = M_min + (M_max - M_min) * float(rand())/RAND_MAX,
	});
}

//----------------------------------------------------------------------------
void OONApp::remove_entity(size_t ndx) //override
{
	SimApp::remove_entity(ndx);

	//-------------------------
	// Adjust references...
	//--------------

	// Focus obj.:
	if (focused_entity_ndx != ~0u) {
		if (focused_entity_ndx > ndx) {
//cerr << "- NOTE: Index of the followed object has changed due to object removal.\n";
			--focused_entity_ndx;
		} else if (focused_entity_ndx == ndx) {
cerr << "- WARNING: The followed object has ceased to exist...\n";
			focused_entity_ndx = ~0u; //!! Don't just fall back to the player!
		}                                 //!! That'd be too subtle/unexpected/unwanted.
	}

	assert(focused_entity_ndx == ~0u || focused_entity_ndx < entity_count());

	// Remove from the view cache, too:
	oon_main_view().delete_cached_shape(ndx);
}


//----------------------------------------------------------------------------
void OONApp::remove_random_body()
{
	auto entities = entity_count();
	if (entities <= 1) { // Leave the player "superglobe", so not just checking for empty()!
//cerr << "No more \"free\" items to delete.\n";
		return;
	}

	auto ndx = 1/*leave the globe!*/ + (size_t) (rand() * (float(entities-1)) / (RAND_MAX + 1));
//cerr << "Deleting object #" << ndx << "...\n";
	assert(ndx < entities); // Note: entity indexes are 0-based
	assert(ndx > 0);        // Note: 0 is the player globe
	remove_entity(ndx);
}

//----------------------------------------------------------------------------
void OONApp::spawn(size_t parent_ndx, unsigned n)
//!! Should not ignore mass!...
//!!??Should gradually become a method of the object itself?
{
if (parent_ndx != player_entity_ndx()) cerr << "- INTERANL: Non-player object #"<<parent_ndx<<" is spawning...\n";

	if (!n) return;

	const auto& parent = const_entity(parent_ndx); // #41: Support inheritance

	backend.audio.play_sound(n == 1 ? snd_plop1 : n <= 10 ? snd_plop2 : snd_plop3 );

	for (size_t i = 0; i < n; ++i) {
		auto ndx = add_random_body_near(player_entity_ndx());
		auto& newborn = entity(ndx);
		newborn.lifetime = Entity::Unlimited;
		newborn.T = parent.T; // #155: Inherit temperature
		newborn.v = parent.v; // 1e5e8be3: Inherit speed
	}
}


//----------------------------------------------------------------------------
//!! An exhaust jet should be created for each thruster!
void OONApp::exhaust_burst(size_t base_ndx/* = 0*/, /*Math::Vector2f thrust_vector,*/ unsigned n/* = ...*/)
{
	static unsigned particles_to_add = appcfg.get("sim/exhaust_particles_add", n);
	static auto     exhaust_density = Phys::DENSITY_ROCK * appcfg.get("sim/exhaust_density_ratio", 0.001f);
	static uint32_t exhaust_color = appcfg.get("sim/exhaust_color", 0xaaaaaa);
	static auto     r_min = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/exhaust_particle_min_size_ratio", 0.02f);
	static auto     r_max = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/exhaust_particle_max_size_ratio", 0.01f);
	static auto     airgap = appcfg.get("sim/exhaust_gap", 2.f);

//cerr <<"DBG> cfg.exhaust_density_ratio: "<< appcfg.get("sim/exhaust_density_ratio", 0.001f) <<'\n';
//cerr <<"DBG> -> exhaust_density: "<< exhaust_density <<'\n';

	static Emitter::Config common_cfg = {
		.eject_velocity = {0, 0},
		.v_factor = appcfg.exhaust_v_factor, //!! Should just be calculated instead!,
		.offset_factor = appcfg.exhaust_offset_factor, //!! Should just be calculated instead!
		.particle_lifetime = appcfg.exhaust_lifetime,
		.create_mass = appcfg.get("sim/exhaust_creates_mass", true),
		.particle_density = exhaust_density,
		.position_divergence = { appcfg.get("sim/exhaust_divergence", 1.f), // Scaled by the radius of the emitter!
						appcfg.get("sim/exhaust_divergence", 1.f) },
		.velocity_divergence = 1.f, //!! Just an exp. "randomness factor" for now!...
		.particle_mass_min = Phys::mass_from_radius_and_density(r_min, Phys::DENSITY_OF_EARTH), //!! WAS: exhaust_density
		.particle_mass_max = Phys::mass_from_radius_and_density(r_max, Phys::DENSITY_OF_EARTH), //!! WAS: exhaust_density
		.color = exhaust_color,
	};

	auto& base = entity(base_ndx); // Not const: will deplete!

	// This "accidentally" creates a nice rainbowish color pattern in the plumes...
	auto adjust_color = [](uint32_t base_color){
		constexpr auto color_spread = (float)0x111111;
		return uint32_t(base_color + color_spread - 2 * color_spread * float(rand())/RAND_MAX);
	};

	//!! This should be calculated from player_thrust_force (around 3e36 N curerently):
	const auto eject_v = 4e9f;// * abs(appcfg.exhaust_v_factor/2); //! Since this is a property of the thrusters, don't
	                                                               //! adjust with the full v-factor!... Just a hint! :)
	Emitter* thruster = nullptr; // Just for uniformity (see below!)...

	if (base.thrust_up.thrust_level()) {
		static Emitter up_thrust_emitter(common_cfg, *this);
		thruster = &up_thrust_emitter;
		thruster->cfg.eject_velocity = {0, -eject_v};
		thruster->cfg.eject_offset = {0, -base.r * airgap};
		thruster->cfg.color = adjust_color(exhaust_color);
		thruster->emit_particles(base_ndx, particles_to_add);
	}
	if (base.thrust_down.thrust_level()) {
		static Emitter dn_thrust_emitter(common_cfg, *this);
		thruster = &dn_thrust_emitter;
		thruster->cfg.eject_velocity = {0, eject_v};
		thruster->cfg.eject_offset = {0, base.r * airgap};
		thruster->cfg.color = adjust_color(exhaust_color);
		thruster->emit_particles(base_ndx, particles_to_add);
	}
	if (base.thrust_left.thrust_level()) {
		static Emitter lt_thrust_emitter(common_cfg, *this);
		thruster = &lt_thrust_emitter;
		thruster->cfg.eject_velocity = {eject_v, 0};
		thruster->cfg.eject_offset = {base.r * airgap, 0};
		thruster->cfg.color = adjust_color(exhaust_color);
		thruster->emit_particles(base_ndx, particles_to_add);
	}
	if (base.thrust_right.thrust_level()) {
		static Emitter rt_thrust_emitter(common_cfg, *this);
		thruster = &rt_thrust_emitter;
		thruster->cfg.eject_velocity = {-eject_v, 0};
		thruster->cfg.eject_offset = {-base.r * airgap, 0};
		// But, instead, use this "alternative engine" here (but only if it "works"!...):
		static SkyPrint loremipsum_skyprinter(appcfg.get("player/test/tagline",
			"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."));
		if (loremipsum_skyprinter.emit()) {
			static Emitter rt_lorem_emitter(common_cfg, *this);
			thruster = &rt_lorem_emitter;
	                thruster->cfg.position_divergence = {0, 0};
			thruster->cfg.velocity_divergence = 0;
			thruster->cfg.eject_offset = {-base.r * airgap, 0}; //!! = {-eject_v/10, 0}; // A gap looks shit here!
			thruster->cfg.eject_velocity = {-eject_v/10, 0}; // To avoid garbled clouds if not moving...
			thruster->cfg.color = adjust_color(exhaust_color);
			thruster->emit_particles(base_ndx, loremipsum_skyprinter.active_pixels, loremipsum_skyprinter.nozzles);
			thruster = nullptr; // Prevent the default action...
		}
	}
}


//----------------------------------------------------------------------------
void OONApp::shield_energize(size_t emitter_ndx, /*Math::Vector2f shoot_vector,*/ unsigned n/* = ...*/)
{
	static auto     particle_density = Phys::DENSITY_ROCK * appcfg.get("sim/shield_density_ratio", 0.001f);
	static uint32_t color = appcfg.get("sim/shield_color", 0xffff99);
	static auto     r_min = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/shield_particle_min_size_ratio", 0.02f);
	static auto     r_max = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/shield_particle_max_size_ratio", 0.01f);
//cerr <<"DBG> cfg.exhaust_density_ratio: "<< appcfg.get("sim/exhaust_density_ratio", 0.001f) <<'\n';
//cerr <<"DBG> -> shield_density: "<< particle_density <<'\n';
	static Emitter thrust_exhaust_emitter(
	Emitter::Config{
		.eject_velocity = {0, 0},
		.v_factor = appcfg.get("sim/shield_v_factor", 0.1f),
		.offset_factor = appcfg.get("sim/shield_offset_factor", 4.f),
		.particle_lifetime = appcfg.get("sim/shield_decay_time", 5.f),
		.create_mass = false, // Disabled: appcfg.get("sim/shield_creates_mass", false),
		.particle_density = particle_density,
		.position_divergence = { appcfg.get("sim/shield_initial_spread", 10.f), // Scaled by the emitter's radius!
		                         appcfg.get("sim/shield_initial_spread", 10.f) },
		.velocity_divergence = appcfg.get("sim/shield_divergence", 1.f), //!! Just an exp. "randomness factor" for now!...
		.particle_mass_min = Phys::mass_from_radius_and_density(r_min, Phys::DENSITY_OF_EARTH),
		.particle_mass_max = Phys::mass_from_radius_and_density(r_max, Phys::DENSITY_OF_EARTH),
		.color = color,
	}, *this);

//	emitter_cfg.eject_velocity = entity(emitter_ndx).v;
	thrust_exhaust_emitter.emit_particles(emitter_ndx, n ? n : appcfg.shield_burst_particles);
}


//----------------------------------------------------------------------------
void OONApp::chemtrail_burst(size_t emitter_ndx/* = 0*/, unsigned n/* = ...*/)
{
	static auto  chemtrail_v_factor      = appcfg.get("sim/chemtrail_v_factor", 0.1f);
	static auto  chemtrail_offset_factor = appcfg.get("sim/chemtrail_offset_factor", 0.2f);
	static float chemtrail_lifetime      = appcfg.get("sim/chemtrail_lifetime", Entity::Unlimited);
	static bool  chemtrail_creates_mass  = appcfg.get("sim/chemtrail_creates_mass", true);
	static auto  chemtrail_density       = Phys::DENSITY_ROCK * appcfg.get("sim/chemtrail_density_ratio", 0.001f);
	static auto  chemtrail_divergence    = appcfg.get("sim/chemtrail_divergence", 1.f);
	static auto  r_min = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/chemtrail_particle_min_size_ratio", 0.02f);
	static auto  r_max = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/chemtrail_particle_max_size_ratio", 0.1f);
	static auto  M_min = Phys::mass_from_radius_and_density(r_min, chemtrail_density);
	static auto  M_max = Phys::mass_from_radius_and_density(r_max, chemtrail_density);

	auto& emitter = entity(emitter_ndx); // Not const: will deplete!
	auto p_range = emitter.r * 5;
	auto v_range = Model::World::CFG_GLOBE_RADIUS * chemtrail_divergence; //!! ...by magic, right? :-/

	auto emitter_old_r = emitter.r;

	for (unsigned i = 0; i++ < n;) {
		auto particle_mass = M_min + (M_max - M_min) * float(rand())/RAND_MAX;
		if (!chemtrail_creates_mass && emitter.mass < particle_mass) {
//cerr << "- Not enough mass to emit particle...\n";
			continue;
		}

		add_entity({
			.lifetime = chemtrail_lifetime,
			.density = chemtrail_density,
			//!!...Jesus, those "hamfixted" pseudo Δts here! :-o :)
			.p = { (rand() * p_range) / RAND_MAX - p_range/2 + emitter.p.x + emitter.v.x * chemtrail_offset_factor,
			       (rand() * p_range) / RAND_MAX - p_range/2 + emitter.p.y + emitter.v.y * chemtrail_offset_factor },
			.v = { (rand() * v_range) / RAND_MAX - v_range/2 + emitter.v.x * chemtrail_v_factor,
			       (rand() * v_range) / RAND_MAX - v_range/2 + emitter.v.y * chemtrail_v_factor },
			.color = (uint32_t) (float)0xffffff * rand(),
			.mass = particle_mass,
		});

		if (!chemtrail_creates_mass) emitter.mass -= particle_mass;
//cerr <<"emitter.mass -= emitter_mass_loss: "<< emitter.mass <<" -= "<< particle_mass <<'\n';
	}

	assert(emitter.mass >= 0);
	emitter.recalc();
	resize_shape(emitter_ndx, float(emitter.r / emitter_old_r));
}


//----------------------------------------------------------------------------
void OONApp::pause_hook(bool)
{
	//!! As a quick hack, time must restart from 0 when unpausing...
	//!! (The other restart() on pausing is redundant; just keeping it simple...)
cerr << "- INTERNAL: Main clock restarted on pause on/off (in the pause-hook)!\n";
	backend.clock.restart();
}


//----------------------------------------------------------------------------
//!! Move this to SimApp, but only together with its counterpart in the update loop!
//!! Note that resetting the iter. counter and the model time should pro'ly be associated
//!! with run(), which should then be non-empty in SimApp, and should also somehow
//!! bring with it some main-loop logic to handle basic chores like time control & stepping!
//!! Perhaps most of that ugly & brittle `update_thread_main_loop()` could be moved there,
//!! and then updates_for_next_frame() could be an app callback (plus some new ones, handling
//!! that Window Context bullshit etc.), and its wrapping in SimApp could hopefully handle the timing stuff.
void OONApp::time_step(int steps)
{
	// Override the loop count limit, if reached (this may not always be applicable tho!); -> #216
	if (iterations.maxed())
		++iterations.limit;

	timestepping = steps; //! See resetting it in updates_for_next_frame()!
}

//----------------------------------------------------------------------------
void OONApp::updates_for_next_frame()
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	//!! I guess this should come before processing the controls, so
	//!! the controls & their follow-up updates are not split across
	//!! time frames -- but I'm not sure if that's actually important!...
	//!! (Note: they're still in the same "rendering frame" tho, that's
	//!! why I'm not sure if this matters at all.)
	//!!
	//!! Also, the frame times should still be tracked (and, as a side-effect, the FPS gauge updated)
	//!! even when paused (e.g. to support [dynamically accurate?] time-stepping etc.)!
/*!!	time.last_frame_delay = time.Δt_since_last_query([](*this){
		auto capture = clock.getElapsedTime().asSeconds();
		clock.restart(); //! Must also be duly restarted on unpausing!
		return capture;
	});
!!*/
	//!! Most of this should be done by Time itself!
	time.last_frame_delay = backend.clock.get();
	time.real_session_time += time.last_frame_delay;
	backend.clock.restart(); //! Must also be restarted on unpausing, because Pause stops it!
	// Update the FPS gauge
	avg_frame_delay.update(time.last_frame_delay);

	//----------------------------
	// Model updates...
	//
	// - Disabled when paused, unless explicitly single-stepping...
	//
	if (!paused() || timestepping) {

		//----------------------------
		//!!? Get some fresh immediate (continuous) input control state updates,
		//!!? in addition to the async. event_loop()!...
		//!! This doesn't just do low-level controls, but "fires" gameplay-level actions!
		//!! (Not any actual processing, just input translation... HOPEFULLY! :) )
		perform_control_actions();

		//----------------------------
		// Determine the size of the next model iteration time slice...
		//
		Time::Seconds Δt;
		if (cfg.fixed_model_dt_enabled) { // "Artificial" fixed Δt for reproducible results, but not frame-synced!
			//!! A fixed dt would require syncing the upates to a real-time clock (balancing/smoothening, pinning etc...) -> #215
			Δt = cfg.fixed_model_dt;
			//!!Don't check: won't be true if changing cfg.fixed_model_dt_enabled at run-time!
			//!!assert(Δt == time.last_model_Δt); // Should be initialized by the SimApp init!
		} else {
			Δt = time.last_model_Δt = time.last_frame_delay;
				// Just an estimate; the last frame time can't guarantee anything about the next one, obviously.
		}

		Δt *= time.scale;
		if (time.reversed || timestepping < 0) Δt = -Δt;

		time.model_Δt_stats.update(Δt);

		//----------------------------
		// Update...
		//
		//!! Move to a SimApp virtual, I guess (so at least the counter capping can be implicitly done there; see also time_step()!):
		if (!iterations.maxed()) {

			update_world(Δt);

			++iterations;

			// Clean-up decayed bodies:
			for (size_t i = player_entity_ndx() + 1; i < entity_count(); ++i) {
				auto& e = entity(i);
				if (e.lifetime != Entity::Unlimited && e.lifetime <= 0) {
					remove_entity(i); // Takes care of "known" references, too!
				}
			}

		} else {
			if (cfg.exit_on_finish) {
				cerr << "Exiting (as requested): iterations finished.\n";
				request_exit();
			}
		}

		//!! Time-stepping should take precedence and prevent immediate exit
		//!! in the "plain finished" case above! Since request_exit() doesn't
		//!! abort/return on its own, it's *implicitly* doing the right thing,
		//!! but that might change to actually aborting later (e.g. via an excpt.)
		//!! -- so, this reminder has been added for that case...

		// One less time-step to make next time (if any):
		if (timestepping) if (timestepping < 0 ) ++timestepping; else --timestepping;
	}

	//----------------------------
	// View adjustments...
	//!
	//! NOTE: MUST COME AFTER CALCULATING THE NEW MODEL STATE!
	//!
	// - Auto-scroll to follow focused player/object
	// - Manual panning
	// - Zoom
	// - Obj Monitor on/off
	// - ...

	ui_gebi(ObjMonitor).active(
		hovered_entity_ndx < entity_count() ||
		focused_entity_ndx < entity_count()
	);

	auto _focus_locked_ = false;
	if (scroll_locked()) {
		// Panning follows focused obj. with locked focus point:
		_focus_locked_ = true;
		if (focused_entity_ndx != ~0u)
			pan_to_focus(focused_entity_ndx);
	} else {
		// Focus point follows focused obj., with panning only if drifting off-screen:
		//!! Should be possible to switch this off!
		if (focused_entity_ndx != ~0u) {
static const float autofollow_margin    = appcfg.get("controls/autofollow_margin", 100.f);
static const float autofollow_throwback = appcfg.get("controls/autofollow_throwback", 2.f);
static const float autozoom_delta       = appcfg.get("controls/autozoom_rate", 0.1f);
			oon_main_camera().focus_offset = oon_main_camera().world_to_view_coord(
				Vector2f(entity(focused_entity_ndx).p));
			if (oon_main_camera().confine(Vector2f(entity(focused_entity_ndx).p),
			    autofollow_margin + autofollow_margin/2 * oon_main_camera().scale()/OONConfig::DEFAULT_ZOOM,
			    autofollow_throwback)) { // true = drifted off
				zoom_control(AutoFollow, -autozoom_delta); // Emulate the mouse wheel...
//cerr << "oon_main_camera().scale(): "<<oon_main_camera().scale()<<", DEFAULT_ZOOM: "<<oon_main_camera().scale()<<", ratio: "<<oon_main_camera().scale() / OONConfig::DEFAULT_ZOOM<<'\n';
			}
		}
	}
	// Update the focus lock indicator:
	sfw::set<sfw::CheckBox>("Pan follows object", _focus_locked_);

	// Update the FPS indicator bar:
	sfw::set<sfw::ProgressBar>("FPS", 1/(float)avg_frame_delay);


	view_control(); // Manual view adjustments

//!!IPROF_SYNC_THREAD;
}

} // namespace OON