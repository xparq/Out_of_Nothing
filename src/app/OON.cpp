#include "OON.hpp"
#include "UI/hud.hpp"

#include "sz/sign.hh"

#include <cstdlib>
	using std::rand; // and the RAND_MAX macro!
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>

using namespace Szim;
using namespace Model;
using namespace Math;
using namespace UI;
using namespace std;


//----------------------------------------------------------------------------
#ifdef DEBUG
static std::string hud_test_callback_string() { return "this is a std::string"; }
static const char* hud_test_callback_ccptr()  { return "this is a const char*"; }
#endif


//----------------------------------------------------------------------------
void OON::init() // override
{
	//!! Currently, the data watcher HUD setup depends directly on the player objects
	//!! that have just been created above, so the UI init CAN'T happen before that...:
	//_setup_UI();

	const auto& w = const_world();
		//!! "Some" model world has been implicitly created by SimApp for now... :-o

	// Add the "Player Superglobe" first
	//!!
	//!! THIS MUST COME BEFORE CALLING add_bodies()! :-o
	//!!
	[[maybe_unused]] auto player_entity_index
		= add_player({.r = w.CFG_GLOBE_RADIUS, .density = Physics::DENSITY_ROCK, .p = {0,0}, .v = {0,0}, .color = 0xffff20});
	assert(entity_count() > player_entity_index);
	assert(player_entity_ndx() == player_entity_index);

	focused_entity_ndx = player_entity_ndx();

	_setup_UI();

	//!! MOVE THE SESSION LOGIC TO SimApp:
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
		// Add 2 "moons" with fixed parameters (mainly for testing):
		add_body({.r = w.CFG_GLOBE_RADIUS/10, .p = {w.CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -w.CFG_GLOBE_RADIUS * 2},
					.color = 0xff2020});
		add_body({.r = w.CFG_GLOBE_RADIUS/7,  .p = {-w.CFG_GLOBE_RADIUS * 1.6f, +w.CFG_GLOBE_RADIUS * 1.2f}, .v = {-w.CFG_GLOBE_RADIUS*1.8, -w.CFG_GLOBE_RADIUS*1.5},
					.color = 0x3060ff});
	}

	// App-level cmdline options (overrides)...
	// Note: the (!!actually: "some"...!!) system-/engine-level options have been processed/applied already!
	try { // <- Absolutely required, as sto...() are very throw-happy.
		// Doing the ones that can't fail first, so an excpt. won't skip them:
		if (args["interact"]) {
			interact_all();
		}; if (args["bodies"]) {
			auto n = stoi(args("bodies")) - 2; // 2 have already been created
			add_bodies(n < 0 ? 0 : n); // Avoid possible overflow!
		}; if (args["friction"]) {
			float f = stof(args("friction"));
			world().FRICTION = f;
		}; if (!args("zoom").empty()) { //!! Should be done by SimApp, and then the audio init
			float factor = stof(args("zoom"));
			if (factor) zoom(factor);
		}
	} catch(...) {
		cerr << __FUNCTION__ ": ERROR processing/applying some cmdline args!\n";
		request_exit(-1);
		return;
	}

	// Audio...
	//!!
	//!! Later, with more mature session mgmt., music loading etc. should NOT
	//!! happen here, ignoring all that!
	//!!
	// Note: --snd=off has been taken care of by the engine already, so it's
	// safe to start playing here no matter what; it will not unmute.
	clack_sound = backend.audio.add_sound(string(cfg.asset_dir + "sound/clack.wav").c_str());
	backend.audio.play_music(cfg.background_music.c_str());
	//backend.audio.play_music(sz::prefix_if_rel(asset_dir, "music/extra sonic layer.ogg"));
}


//----------------------------------------------------------------------------
void OON::done() // override
{
//	cerr << __FUNCTION__ << ": Put any 'onExit' tasks (like saving the last state) here!...\n";

	//!! MOVE THE SESSION LOGIC TO SimApp:
	// Let the session-manager auto-save the current session (unless disabled with --session-no-save; see SimApp::init()!)
	if (args["session"]) { // If empty, it will be saved as "UNNAMED.autosave" or sg. like that...
		session.close();
	}
}

//----------------------------------------------------------------------------
#ifndef DISABLE_HUD
void OON::toggle_huds()  { _show_huds = !_show_huds; }
bool OON::huds_active()  { return _show_huds; }
void OON::toggle_help()  { ui_gebi(HelpPanel).active(!ui_gebi(HelpPanel).active()); }
#endif

//----------------------------------------------------------------------------
void OON::_setup_UI()
{
	using namespace sfw;
	// The SFW GUI is used as a translucent overlay, so an alpha-enabled bgColor
	// must be applied. The clearBackground option must be left at its default (true):
	//Theme::clearBackground = false;
	Theme::click.textColor = sfw::Color("#ee9"); //!! "input".textColor... YUCK!! And "click" for LABELS?!?!
	gui.setPosition(10, cfg.WINDOW_HEIGHT-200);
	auto form = gui.add(new Form, "Params");
		form->add("Show HUDs", new CheckBox([&](auto*){ this->toggle_huds(); }, huds_active()));
		form->add("Fixed model Δt", new CheckBox([&](auto*){ this->toggle_fixed_model_dt(); },
		                                         cfg.fixed_model_dt_enabled));
		form->add("Pan override", new CheckBox); // Will be polled by the control loop!
		form->add(" - pan locked", new CheckBox)->disable(); // Will be updated by the ctrl. loop!
	gui.recall("Show HUDs")->setTooltip("Press [?] to toggle the Help panel");

	auto volrect = gui.add(new Form, "VolForm");
	volrect->add("Volume", new Slider({/*.orientation = Vertical*/}, 70), "volume slider")
		->setCallback([&](auto* w){backend.audio.volume(w->get());})
		->update(75); // %
	auto audio_onoff = gui.add(new Form, "AudioOnOffForm");
	audio_onoff->add("Audio: ", new CheckBox([&](auto*){backend.audio.toggle_audio();}, backend.audio.enabled));
	audio_onoff->add(" - FX: ", new CheckBox([&](auto*){backend.audio.toggle_sounds();}, backend.audio.fx_enabled));

#ifndef DISABLE_HUD
	//!!?? Why do all these member pointers just work, also without so much as a warning,
	//!!?? in this generic pointer passing context?!
	//!!
	//!! "Evenfurthermore": why do all these insane `this` captures apparently survive
	//!! all the obj recreation shenanigans (they *are* recreated, right??...) after
	//!! a World reload?!?!?!
	//!!

	//! This still needs to be a double nested lambda: the outer wrapper "converts" the
	//! signature to match the string-returning nullary functions the HUD stuff supports.
	auto ftos = [](auto* ptr_x) { return [ptr_x]() { static constexpr size_t LEN = 15;
			char buf[LEN + 1]; auto [ptr, ec] = std::to_chars(buf, buf+LEN, *ptr_x);
			return string(ec != std::errc() ? "???" : (*ptr = 0, buf));
		};
	};

	auto& debug_hud = ui_gebi(PlayerData);

#ifdef DEBUG
//!!Should be Rejected compile-time (with a static_assert):
//!! - well, rejected indeed, but only "fortunately", and NOT by my static_assert!... :-/
//!!	debug_hud << "DBG>" << string("debug");
	static const auto const_debug = "CONST STRING "s;
	debug_hud << "DBG>" << &const_debug;
//!!shouldn't compile:	debug_hud << "DBG>" << const_debug;
	static auto debug = "STR "s;
	debug_hud << "DBG>" << &debug;
//!!shouldn't compile:	debug_hud << "DBG>" << debug;
	debug_hud << "\nHŐTŰRŐ lótúró [αβ°C]" << 1e300 << "\n";

	debug_hud << "\n"
	<< "test fn->string: " << hud_test_callback_string << "\n"
	<< "test fn->ccptr: " << hud_test_callback_ccptr << "\n"
	<< "test λ->ccptr: " << []{ return "autoconv to string?..."; } << "\n"
//!!NOPE:	<< "test λ->int: " << []{ return 0xebba; } << "\n"
//!!NOPE:	<< "test λ->float: " << []{ return 12.345; } << "\n"
	// By-value data that are not closures:
	<< "test val. string: " << "temp"s << "\n"
	<< "test val. int: " << 12345 << "\n"
	<< "test val. float: " << 12.345f << "\n"
	<< "test val. double: " << 1e300 << "\n"
//!!NOT YET:	<< "test val. bool: " << true << "\n"
	<< "\n";
#endif

	debug_hud
		<< "# of objs.: " << [=](){ return to_string(this->entity_count()); }
		<< "\nBody interactions: " << &this->const_world()._interact_all
		<< "\nDrag: " << ftos(&this->const_world().FRICTION)
		<< "\n"
	;
	//! Setting up player data watches below requires that the player
	//! entity have actually been created already!
	//! If not, an assertion will fail, but only in the DEBUG build!
	//! So... Just checking it run-time, too, as I have made this mistake
	//! too many times now... :)
if ( !(player_entity_ndx() < entity_count()) ) {
	cerr << "- INTERNAL ERROR: UI/PlayerHUD init before player entity init!\n";
} else {
	debug_hud
		<< "\nPlayer #1:"
		<< "\n"
		<< "\n  R: " << ftos(&this->player_entity().r) //!!#29: &(world().CFG_GLOBE_RADIUS) // OK now, probably since c365c899
		<< "\n  T: " << ftos(&this->player_entity().T)
		<< "\n  M: " << ftos(&this->player_entity().mass)
		<< "\n  x: " << ftos(&this->player_entity().p.x)
		<<   ", y: " << ftos(&this->player_entity().p.y)
		<< "\n  vx: " << ftos(&this->player_entity().v.x)
		<<   ", vy: " << ftos(&this->player_entity().v.y)
		<< "\n"
	;
}
	debug_hud
		<< "\nCAMERA: "
		<< "\n  X: " << &view.offset.x << ", Y: " << &view.offset.y
		<< "\n  ZOOM: " << &view.scale
	;
/*	debug_hud
		<< "\n"
		<< "\nSHIFT" << (bool*)&_kbd_state[SHIFT]);
		<< "\nLSHIFT" << (bool*)&_kbd_state[LSHIFT]);
		<< "\nRSHIFT" << (bool*)&_kbd_state[RSHIFT]);
		<< "\nCAPS LOCK" << (bool*)&_kbd_state[CAPS_LOCK]);
		<< "\nSCROLL LOCK" << (bool*)&_kbd_state[SCROLL_LOCK]);
		<< "\nNUM LOCK" << (bool*)&_kbd_state[NUM_LOCK]);
	;
*/
//	debug_hud << "\n"
//	          << "\nPress ? for help...";

	//------------------------------------------------------------------------
	auto& timing_hud = ui_gebi(TimingStats);
	timing_hud
		<< "FPS: " << [=](){ return to_string(1 / (float)this->avg_frame_delay); }
		<< "\nlast frame Δt: " << [=](){ return to_string(this->time.last_frame_delay * 1000.0f) + " ms"; }
		<< "\nmodel Δt: " << [=](){ return to_string(this->time.last_model_Δt * 1000.0f) + " ms"; }
		<<            " " << [=](){ return cfg.fixed_model_dt_enabled ? "(fixed)" : ""; }
		<< "\ncycle: " << [=](){ return to_string(iterations); }
		<< "\nReal elapsed time: " << &time.real_session_time
	//!!??WTF does this not compile? (It makes no sense as the gauge won't update, but regardless!):
	//!!??  << vformat("frame dt: {} ms", time.last_frame_delay)
		<< "\nTime reversed: " << &time.reversed
		<< "\nTime scale: " << ftos(&this->time.scale)
		<< "\nModel timing stats:"
//		<< "\n    updates: " << &time.model_Δt_stats.samples
		<< "\n    total t: " << &time.model_Δt_stats.total
		<< "\n  Δt:"
		<< "\n    last: " << &time.model_Δt_stats.last
		<< "\n    min abs: " << &time.model_Δt_stats.umin
		<< "\n    max abs: " << &time.model_Δt_stats.umax
		<< "\n    min: " << &time.model_Δt_stats.min
		<< "\n    max: " << &time.model_Δt_stats.max
		<< "\n    avg.: " << [=]{ return to_string(this->time.model_Δt_stats.average());}
	;
//cerr << timing_hud;

	//------------------------------------------------------------------------
	auto& help_hud = ui_gebi(HelpPanel);
	help_hud
        //	<< "----------- Controls:\n"
		<< "Arrows:     Thrust\n"
		<< "Space:      \"Exhaust\" sprinkle\n"
		<< "Ins:        Spawn 100 objects (+Shift: only 1)\n"
		<< "Del:        Remove 100 objects (+Shift: only 1)\n"
        //	<< "----------- Metaphysics:\n"
		<< "Tab:        Toggle object interactions\n"
		<< "F:          Decrease (+Shift: incr.) friction\n"
        //	<< "C:          chg. collision mode: pass/stick/bounce\n"
		<< "------------ Time Control:\n"
		<< "Pause or H: Toggle halting the physics (time)\n"
		<< "Enter:      Step 1 time slice forward\n"
		<< "Backspace:  Step 1 time slice backward\n"
		<< "R:          Reverse time\n"
		<< "T:          Time accel. (+Shift: decel.)\n"
		<< "X:          Toggle fixed Δt for model updates\n"
		<< "------------ View:\n"
		<< "A W S D:     Pan\n"
		<< "+/- or Mouse Wheel: Zoom\n"
		<< "Shift:       Pin scroll to player (or other focused entity)\n"
		<< "Scroll Lock: Toggle locked auto-scroll\n"
		<< "Left Click:  Set focused obj (or empty point as zoom center)\n"
		<< "             +Shift: Set focus & pin player or clicked obj\n"
		<< "Home:        Home in on the player\n"
		<< "Ctrl+Home:   Reset view to Home pos. (not the zoom)\n"
		<< "Ctrl+Alt:    Leave movement trails while holding\n"
		<< "---------- Admin:\n"
		<< "F1-F4:     Save world snapshots (+Shift: load)\n"
		<< "M:         Mute/unmute music, N: sound fx\n"
		<< "Shift+M:   Mute/unmute all audio\n"
		<< "Shift+P:   Toggle FPS throttling (lower CPU load)\n"
		<< "F11:       Toggle fullscreen\n"
		<< "F12:       Toggle HUDs\n"
		<< "\n"
		<< "Esc:       Quit\n"
		<< "\n"
		<< "Command-line options: " << args.exename() << " /?"
	;
//cerr << help_hud;

	help_hud.active(cfg.get("show_help_on_start", true));
#endif
}

//----------------------------------------------------------------------------
unsigned OON::add_player(World::Body&& obj)
{
	// These are the player modelling differences:
	obj.add_thrusters();
	obj.superpower.gravity_immunity = true;
	obj.superpower.free_color = true;
	obj/*.superpower*/.lifetime = World::Body::Unlimited; //!!?? Should this be a superpower instead?

	return (unsigned) //!! Blatant narrowing conv., hoping entity_count() will never overflow `unsigned`...
		add_body(std::forward<World::Body>(obj));
}

void OON::remove_player(unsigned)
{
}



//----------------------------------------------------------------------------
bool OON::poll_and_process_controls()
{
	bool action = false;
	if (_ctrl_update_thrusters()) {
		action = true;
//		if (keystate(SPACE)) {
//			action = true;
//			exhaust_burst(player_entity_ndx(), 5);
//		}
	}
	// Allow this now irrespective of any engine firing:
	if (keystate(SPACE)) {
		action = true;
		exhaust_burst(player_entity_ndx(), 5);
	}

	if (_ctrl_update_pan())
		action = true;

	return action;
}

//----------------------------------------------------------------------------
void OON::up_thruster_start()    { entity(player_entity_ndx()).thrust_up.thrust_level(cfg.THRUST_FORCE); }
void OON::down_thruster_start()  { entity(player_entity_ndx()).thrust_down.thrust_level(cfg.THRUST_FORCE); }
void OON::left_thruster_start()  { entity(player_entity_ndx()).thrust_left.thrust_level(cfg.THRUST_FORCE); }
void OON::right_thruster_start() { entity(player_entity_ndx()).thrust_right.thrust_level(cfg.THRUST_FORCE); }
void OON::up_thruster_stop()     { entity(player_entity_ndx()).thrust_up.thrust_level(0); }
void OON::down_thruster_stop()   { entity(player_entity_ndx()).thrust_down.thrust_level(0); }
void OON::left_thruster_stop()   { entity(player_entity_ndx()).thrust_left.thrust_level(0); }
void OON::right_thruster_stop()  { entity(player_entity_ndx()).thrust_right.thrust_level(0); }

bool OON::_ctrl_update_thrusters()
{
	auto drv = false;
	if (keystate(UP))    { drv = true;    up_thruster_start(); } else    up_thruster_stop();
	if (keystate(DOWN))  { drv = true;  down_thruster_start(); } else  down_thruster_stop();
	if (keystate(LEFT))  { drv = true;  left_thruster_start(); } else  left_thruster_stop();
	if (keystate(RIGHT)) { drv = true; right_thruster_start(); } else right_thruster_stop();
	return drv;
}

//----------------------------------------------------------------------------
void OON::pan_reset()
{
	view.offset = {0, 0};

	// Since the player entity may have moved out of view, stop focusing on it:
	//!!
	//!!?? What is the rule for Scroll Lock in this case?
	//!!The key should be turned off!...
	//!!
	focused_entity_ndx = ~0u; //!!... Whoa! :-o See updates_for_next_frame()!
}

void OON::pan(Vector2f delta) { view.pan_x(delta.x); view.pan_y(delta.y); }
void OON::pan_x(float delta)  { view.pan_x(delta); }
void OON::pan_y(float delta)  { view.pan_y(delta); }

void OON::center_to_entity(size_t id)
{
	view.offset = entity(id).p * view.scale;
	view.focus_offset = view.offset; //!!??
}

void OON::center_to_player(unsigned player_id)
{
	assert(player_id == 1);
	focused_entity_ndx = player_entity_ndx(player_id);
	center_to_entity(focused_entity_ndx);
}


void OON::follow_entity(size_t id)
{
	auto vpos = view.world_to_view_coord(entity(id).p);
	view.offset += vpos - view.focus_offset;
}

void OON::follow_player(unsigned player_id)
{
	assert(player_id == 1);
	auto new_player_vpos = view.world_to_view_coord(player_entity(player_id).p);
	view.offset += new_player_vpos - view.focus_offset;
}


void OON::zoom_reset() { view.scale = SimAppConfig::DEFAULT_ZOOM; } //!!...
void OON::zoom(float factor)
{
//!!pre_zoom_hook(factor);
	view.zoom(factor);
	post_zoom_hook(factor);
}
// These can't call view.zoom_in/out directly, because we need to trigger the zoom_hook above!...:
void OON::zoom_in () { zoom(1.f + CFG_ZOOM_CHANGE_RATIO); }
void OON::zoom_out() { zoom(1.f / (1.f + CFG_ZOOM_CHANGE_RATIO)); }


/*!!
void OON::zoom(float factor)
{
//auto viewpos = view.world_to_view_coord(player_entity().p);
//cerr << "- focus vs player diff: " << (viewpos - view.focus_offset).x << ", " << (viewpos - view.focus_offset).y << '\n';

//!!pre_zoom_hook(factor);
	// Compensate for zoom displacement when the player object is not centered
	auto v = view.world_to_view_coord(view.offset);
	pan((view.focus_offset - v) * view.zoom/factor);
//	auto viewpos = view.focus_offset + view.offset;
//	pan(viewpos - viewpos/factor);

//	auto vpos = view.world_to_view_coord(view.offset);
//	pan(view.focus_offset/factor);

	view.zoom *= factor;

	post_zoom_hook(factor);
}
!!*/


bool OON::_ctrl_update_pan()
{
	auto action = false;

	if (keystate(W)) { action = true; pan_step_y = -CFG_PAN_STEP; }
	if (keystate(S)) { action = true; pan_step_y =  CFG_PAN_STEP; }
	if (keystate(A)) { action = true; pan_step_x = -CFG_PAN_STEP; }
	if (keystate(D)) { action = true; pan_step_x =  CFG_PAN_STEP; }

	if (!action) {
		if (pan_step_x) pan_step_x -= sz::sign(pan_step_x);
		if (pan_step_y) pan_step_y -= sz::sign(pan_step_y);
	}
	if (pan_step_x) pan_x(pan_step_x);
	if (pan_step_y) pan_y(pan_step_y);

	return action;
}

/*
bool OON::_ctrl_driving()
{
	return ((keystate(UP) || keystate(UI::DOWN) || keystate(LEFT) || keystate(RIGHT) ||
			 keystate(W) || keystate(UI::S) || keystate(A) || keystate(D));
}
*/

//----------------------------------------------------------------------------
//!!Move chores like this to Szim API!
void OON::toggle_muting() { backend.audio.toggle_audio(); }
void OON::toggle_music() { backend.audio.toggle_music(); }
void OON::toggle_sound_fx() { backend.audio.toggle_sounds(); }


//----------------------------------------------------------------------------
void OON::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
//	if (!obj1->is_player())
//		obj1->color += 0x3363c3;
}

//----------------------------------------------------------------------------
bool OON::touch_hook(World* w, World::Body* obj1, World::Body* obj2)
{w;
	if (obj1->is_player() || obj2->is_player()) {
		backend.audio.play_sound(clack_sound);
	}

	obj1->T += 100;
	obj2->T += 100;

	obj1->recalc();
	obj2->recalc();

	return false; //!!Not yet used!
}


//----------------------------------------------------------------------------
size_t OON::add_body(World::Body&& obj)
{
	return world().add_body(std::forward<decltype(obj)>(obj));
}

//----------------------------------------------------------------------------
void OON::remove_body(size_t ndx)
{
	world().remove_body(ndx);
}

//----------------------------------------------------------------------------
size_t OON::add_body()
{
	const auto& cw = const_world();
	auto constexpr r_min = cw.CFG_GLOBE_RADIUS / 9;
	auto constexpr r_max = cw.CFG_GLOBE_RADIUS * 3;
	auto constexpr p_range = cw.CFG_GLOBE_RADIUS * 30;
	auto constexpr v_range = cw.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
//cerr << "Adding new object #" << cw.bodies.size() + 1 << "...\n";
	const auto& player = const_entity(player_entity_ndx());
	return add_body({
		.r = (float) (((float)rand() * (r_max - r_min)) / RAND_MAX ) //! suppress warning "conversion from double to float..."
				+ r_min,
		.p = { (rand() * p_range) / RAND_MAX - p_range/2 + player.p.x,
		       (rand() * p_range) / RAND_MAX - p_range/2 + player.p.y },
		.v = { (rand() * v_range) / RAND_MAX - v_range/2 + player.v.x * 0.05f,
		       (rand() * v_range) / RAND_MAX - v_range/2 + player.v.y * 0.05f },
		.color = 0xffffff & ((uint32_t) rand() * rand()),
	});
}

//----------------------------------------------------------------------------
void OON::remove_body()
{
	auto entities = entity_count();
	if (entities < 2) { // Leave the player "superglobe", so not just checking for empty()!
//cerr << "No more \"free\" items to delete.\n";
		return;
	}

	auto ndx = 1/*leave the globe!*/ + rand() * ((entities-1) / (RAND_MAX + 1));
//cerr << "Deleting object #"	 << ndx << "...\n";
	assert(ndx < entities); // Note: entity indexes are 0-based
	assert(ndx > 0);        // Note: 0 is the player globe
	remove_body(ndx);
}

//----------------------------------------------------------------------------
void OON::add_bodies(size_t n)
{
	while (n--) add_body();
}

//----------------------------------------------------------------------------
void OON::remove_bodies(size_t n/* = -1*/)
{
	if (n == (unsigned)-1) n = entity_count();
	while (n--) remove_body();
}


//----------------------------------------------------------------------------
void OON::spawn(size_t parent_ndx, size_t n)
//!!??Should gradually become a method of the object itself
{
if (parent_ndx != player_entity_ndx()) cerr << "- INTERANL: Non-player object #"<<parent_ndx<<" is spawning...\n";

	// -> #41: Enable inheritance
	const auto& parent = const_entity(parent_ndx);
	for (size_t i = 0; i < n; ++i) {
		auto ndx = add_body();
		auto& newborn = entity(ndx);
		newborn.T = parent.T; // #155: Inherit T
	}
}

//----------------------------------------------------------------------------
void OON::exhaust_burst(size_t entity/* = 0*/, size_t n/* = 50*/)
{
/*!! Too boring with all these small particles:
	auto constexpr r_min = const_world().CFG_GLOBE_RADIUS / 10;
	auto constexpr r_max = const_world().CFG_GLOBE_RADIUS * 0.3;
	auto constexpr p_range = const_world().CFG_GLOBE_RADIUS * 2;
	auto constexpr v_range = const_world().CFG_GLOBE_RADIUS * 3; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
*/
	const auto& cw = const_world();
	auto constexpr r_min = cw.CFG_GLOBE_RADIUS / 10;
	auto constexpr r_max = cw.CFG_GLOBE_RADIUS * 0.5;
	auto constexpr p_range = cw.CFG_GLOBE_RADIUS * 5;
	auto constexpr v_range = cw.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
	const auto& rocket = const_entity(entity);
	for (int i = 0; i++ < n;) {
		add_body({
			.r = (float) ((rand() * (r_max - r_min)) / RAND_MAX ) //! Suppress warning "conversion from double to float..."
					+ r_min,
 			//!!...Jesus, those literal pseudo dts here! :-o :)
			.p = { (rand() * p_range) / RAND_MAX - p_range/2 + rocket.p.x - rocket.v.x * 0.1f,
			       (rand() * p_range) / RAND_MAX - p_range/2 + rocket.p.y - rocket.v.y * 0.1f },
			.v = { (rand() * v_range) / RAND_MAX - v_range/2 + rocket.v.x * 0.1f,
			       (rand() * v_range) / RAND_MAX - v_range/2 + rocket.v.y * 0.1f },
			.color = (uint32_t) (float)0xffffff * rand(),
		});
	}
}
