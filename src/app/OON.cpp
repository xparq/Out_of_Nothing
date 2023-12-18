#include "OON.hpp"

#include "Engine/Backend/HCI.hpp"

#include "sfw/GUI.hpp" //!! Used to be in OON_sfml only, but since scroll_locked() requires it...
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

namespace OON {


//----------------------------------------------------------------------------
void OONApp::show_cmdline_help(const Args& args, const char* banner)
{
	banner = "\"Out of Nothing\" - experimental simulation toy\n";

	SimApp::show_cmdline_help(args, banner);

	cout << R"(
  -C cfgfile
          Load config. from 'cfgfile'. If not found, abort.
	  If omitted, ./default.cfg is tried, and if that doesn't exist,
	  internal hardcoded defaultw will be used as a fallback.

  ...lots more to be documented here, sorry!
)";
}


//============================================================================
//============================================================================
namespace _internal {
FUCpp_CameraViewPack::_oon_view_and_cam_container::_oon_view_and_cam_container()
//!!: oon_main_camera({.width  = (float)backend.hci.window().width, //!!WAS: Szim::SimAppConfig::VIEWPORT_WIDTH, //!! Would (should!) be reset later from "real data" from the backend anyway...
//!!                   .height = (float)backend.hci.window().height, //!!WAS: Szim::SimAppConfig::VIEWPORT_HEIGHT,
: _oon_main_camera({.width  = Szim::SimAppConfig::VIEWPORT_WIDTH, //!! Supposed to be reset with "real data" by SimApp anyway...
                    .height = Szim::SimAppConfig::VIEWPORT_HEIGHT,
                    .base_scale = SimAppConfig::DEFAULT_ZOOM})
, _oon_main_view({.width = Szim::SimAppConfig::VIEWPORT_WIDTH,
                  .height = Szim::SimAppConfig::VIEWPORT_HEIGHT},
                  _oon_main_camera)
{
//cerr << "DBG> _oon_view_and_cam_container: _oon_main_view.camera ptr: "<<&_oon_main_view.camera()<<"\n";
}
}

//============================================================================
OONApp::OONApp(int argc, char** argv)
	: FUCpp_CameraViewPack()
	, SimApp(argc, argv, oon_main_view())
	, appcfg(cfg, args) //!! appcfg(SimApp::syscfg)
{
//!! This shouldn't be needed, the engine should take care of it:
//!! And the view resize should also implicitly take care of any camera adjustments, too, so
//!! this commented part would be the one that's actually needed, and the cam. stuff deleted below!
//!!	oon_main_view().resize((float)backend.hci.window().width,
//!!	                         (float)backend.hci.window().height);
	oon_main_camera().resize((float)backend.hci.window().width,
	                         (float)backend.hci.window().height);
}

//----------------------------------------------------------------------------
void OONApp::init() // override
{
	//!! Currently, the data watcher HUD setup depends directly on the player objects
	//!! that have just been created above, so the UI init CAN'T happen before that...:
	//_setup_UI();

	const auto& w = const_world();
		//!! "Some" model world has been implicitly created by SimApp for now... :-o

	// Add the "Player Superglobe" first
	//!!
	//!! THIS MUST COME BEFORE CALLING add_random_bodies_near(player)! :-o
	//!!
	[[maybe_unused]] auto player_entity_index
		= add_player({.r = w.CFG_GLOBE_RADIUS, // Will be recalculated anyway...
		              .density = appcfg.get("sim/player_globe_density", Physics::DENSITY_OF_EARTH / 10.f),
		              .p = {0,0}, .v = {0,0},
		              .color = 0xffff20,
		              .mass = appcfg.get("sim/player_globe_mass", 50 * Physics::MASS_OF_EARTH)});
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
					.color = 0xff2020, .mass = 3e24f});
		add_body({.r = w.CFG_GLOBE_RADIUS/7,  .p = {-w.CFG_GLOBE_RADIUS * 1.6f, +w.CFG_GLOBE_RADIUS * 1.2f}, .v = {-w.CFG_GLOBE_RADIUS*1.8, -w.CFG_GLOBE_RADIUS*1.5},
					.color = 0x3060ff, .mass = 3e24f});
	}

	// App-level cmdline options (overrides)...
	// Note: the (!!actually: "some"...!!) system-/engine-level options have been processed/applied already!
	try { // <- Absolutely required, as sto...() are very throw-happy.
		// Doing the ones that can't fail first, so an excpt. won't skip them:
		if (appcfg.get("sim/global_interactions",
		                                          cfg.global_interactions)) { //!! :-/ EHH, RESOLVE THIS compulsory defult misery!
			interact_all();
		}; if (args["bodies"]) {
			auto n = stoi(args("bodies")) - 2; // 2 have already been created above
			add_random_bodies_near(player_entity_ndx(), n < 0 ? 0 : n); //! Dodge a possible overflow of n
		}; if (args["friction"]) {
			float f = stof(args("friction"));
			world().friction = f;
		}; if (!args("zoom").empty()) { //!! Should be done by SimApp, and then the audio init
			float factor = stof(args("zoom"));
			if (factor) zoom_reset(factor);
		}
	} catch(...) {
		cerr << __FUNCTION__ ": ERROR processing/applying some cmdline args!\n";
		request_exit(-1);
		return;
	}

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
} // init


//----------------------------------------------------------------------------
void OONApp::done() // override
{
//	cerr << __FUNCTION__ << ": Put any 'onExit' tasks (like saving the last state) here!...\n";

	//!! MOVE THE SESSION LOGIC TO SimApp:
	// Let the session-manager auto-save the current session (unless disabled with --session-no-autosave; see SimApp::init()!)
	if (args["session"]) { // If empty and no --session-save-as, it will be saved as "UNNAMED.autosave" or sg. like that.
		session.close();
	}
}

//----------------------------------------------------------------------------
#ifndef DISABLE_HUD
void OONApp::toggle_huds()  { _show_huds = !_show_huds; }
bool OONApp::huds_active()  { return _show_huds; }
void OONApp::toggle_help()  { ui_gebi(HelpPanel).active(!ui_gebi(HelpPanel).active()); }
#endif

//----------------------------------------------------------------------------
void OONApp::_setup_UI()
{
	using namespace sfw;
	// The SFW GUI is used as a translucent overlay, so an alpha-enabled bgColor
	// must be applied. The clearBackground option must be left at its default (true):
	//Theme::clearBackground = false;
	Theme::click.textColor = sfw::Color("#ee9"); //!!("input".textColor!) YUCK!! Also "click" for LABELS?!?!
	auto gui_main_hbox = gui.add(new HBox);

	// Misc UI controls...
	auto left_vbox = gui_main_hbox->add(new VBox);
		auto	gui_form = left_vbox->add(new Form, "Params");
			gui_form->add("Show HUDs", new CheckBox([&](auto*){ this->toggle_huds(); }, huds_active()));
			gui.recall("Show HUDs")->setTooltip("Press [?] to toggle the Help panel");
		auto	volrect = left_vbox->add(new Form, "VolForm");
			volrect->add("Volume", new Slider({/*.orientation = Vertical*/}, 70), "volume slider")
			->setCallback([&](auto* w){backend.audio.volume(w->get());})
			->update(75); // %
		auto	audio_onoff = left_vbox->add(new Form, "AudioOnOffForm");
			audio_onoff->add("Audio: ", new CheckBox([&](auto*){backend.audio.toggle_audio();}, backend.audio.enabled));
			audio_onoff->add(" - FX: ", new CheckBox([&](auto*){backend.audio.toggle_sounds();}, backend.audio.fx_enabled));

	// View...
	gui_main_hbox->add(new Label(" ")); // Just a vert. spacer
	auto	view_form = gui_main_hbox->add(new Form);
		view_form->add("Pan follow object", new CheckBox)->disable(); // Will be updated by the ctrl. loop!
		view_form->add("  - forced follow", new CheckBox); // Will be polled by the control loop!
		view_form->add("Grid lines", new CheckBox([&](auto* w){oon_main_camera().cfg.gridlines = w->get();}, oon_main_camera().cfg.gridlines));

	// Physics tweaking...
	gui_main_hbox->add(new Label(" ")); // Just a vert. spacer
	auto	phys_form = gui_main_hbox->add(new Form);
		auto g_select = new OptionsBox<World::GravityMode>();
			g_select->add("Normal", World::GravityMode::Normal);
			g_select->add("Skewed", World::GravityMode::Skewed);
			g_select->add("Off",    World::GravityMode::Off);
			//g_select->set(World::GravityMode::Off);
			g_select->setCallback([&](auto* w){ this->world().gravity_mode = w->get(); });
		phys_form->add("Gravity mode", g_select);
		phys_form->add(" - bias", new sfw::Slider({.range={-3.0, 3.0}, .step=0, .orientation=sfw::Horizontal}, 80))
			->setCallback([&](auto* w){ this->world().gravity = Model::Physics::G * pow(10.f, w->get()); })
			->set(0);
		phys_form->add("Friction", new sfw::Slider({.range={-1.0, 1.0}, .step=0, .orientation=sfw::Horizontal}, 80))
			->setCallback([&](auto* w){ this->world().friction = w->get(); })
			->set(world().friction);
		phys_form->add("Fixed model Δt", new CheckBox(
			[&](auto*){ this->toggle_fixed_model_dt(); }, cfg.fixed_model_dt_enabled));

	// Save/load...
	gui_main_hbox->add(new Label(" ")); // just a vert. spacer
	auto	saveload_form = gui_main_hbox->add(new Form);
		saveload_form->add("File", new TextBox);
		auto	saveload_buttons = saveload_form->add("", new HBox);
			saveload_buttons->add(new Button("Save"))
				->setTextColor(sf::Color::Black)->setColor(sfw::Color("#f002"))
				->setCallback([&]{
					if (auto* fname_widget = (TextBox*)gui.recall("File"); fname_widget) {
						auto fname = fname_widget->get();
						this->save_snapshot(fname.empty() ? "UNTITLED.save" : fname.c_str());
					}
				});
			saveload_buttons->add(new Button("Load"))
				->setTextColor(sf::Color::Black)->setColor(sfw::Color("#0f02"))
				->setCallback([&]{
					if (auto* fname_widget = (TextBox*)gui.recall("File"); fname_widget) {
						auto fname = fname_widget->get();
						this->load_snapshot(fname.empty() ? "UNTITLED.save" : fname.c_str());
					}
				});

	// Only position after built, so it has its dimensions:
	gui.setPosition(4, cfg.WINDOW_HEIGHT - gui.getSize().y - 4);
		//!! For that 4 above: sfw is still too lame for styling margins/padding... :-/
		//!! Not even this would do anything, actually: ->setPosition({100, -200});

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

	//------------------------------------------------------------------------
	// Timing
	ui_gebi(TimingStats)
		<< "FPS: " << [&](){ return to_string(1 / (float)this->avg_frame_delay); }
		<< "\nlast frame Δt: " << [&](){ return to_string(this->time.last_frame_delay * 1000.0f) + " ms"; }
		<< "\nmodel Δt: " << [&](){ return to_string(this->time.last_model_Δt * 1000.0f) + " ms"; }
		<<            " " << [&](){ return cfg.fixed_model_dt_enabled ? "(fixed)" : ""; }
		<< "\ncycle: " << [&](){ return to_string(iterations); }
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
		<< "\n    avg.: " << [&]{ return to_string(this->time.model_Δt_stats.average());}
	;
//cerr << timing_hud;

	//------------------------------------------------------------------------
	// World
	ui_gebi(WorldData)
		<< "# of objs.: " << [&](){ return to_string(this->entity_count()); }
		<< "\nBody interactions: " << &this->const_world()._interact_all
		<< "\nGravity mode: " << [&](){ return to_string((unsigned)this->const_world().gravity_mode); }
		<< "\n  - strength: " << &this->const_world().gravity
		<< "\nDrag: " << ftos(&this->const_world().friction)
		<< "\n"
	;

	//------------------------------------------------------------------------
	// View
	ui_gebi(ViewData)
		<< "MAIN CAMERA:"
		<< "\n  X: " << &oon_main_camera().offset.x << ", Y: " << &oon_main_camera().offset.y
		<< "\n  ZOOM: " << [&](){ return to_string(this->oon_main_camera().scale()); }
		<< "\n  focus pt: "<< &oon_main_camera().focus_offset.x << ", " << &oon_main_camera().focus_offset.y
/*		<< "\nVIEWPORT:"
		<< "\n_edge_x_min: "<< &oon_main_camera()._edge_x_min
		<< "\n_edge_x_min: "<< &oon_main_camera()._edge_x_max
		<< "\n_edge_y_min: "<< &oon_main_camera()._edge_y_min
		<< "\n_edge_y_min: "<< &oon_main_camera()._edge_y_max
*/	;

//	???_hud << "\nPress ? for help...";

	//------------------------------------------------------------------------
	// "Object Observer"

	//! Setting up player data watches below requires that the player
	//! entity have actually been created already!
	//! If not, an assertion will fail, but only in the DEBUG build!
	//! So... Just checking it run-time, too, as I have made this mistake
	//! too many times now... :)
  if ( !(player_entity_ndx() < entity_count()) ) {
	cerr << "- INTERNAL ERROR: UI/PlayerHUD init before player entity init!\n";
  } else {

	ui_gebi(ObjectData)
		<< [&]{	if (this->focused_entity_ndx == ~0u) return "<NOTHING>"s;
			if (this->focused_entity_ndx >= this->entity_count()) return "INVALID ENTITY #"s + to_string(this->focused_entity_ndx);
			if (this->focused_entity_ndx == player_entity_ndx()) return "Player #"s + to_string(player_entity_ndx());
			else return "Object #"s + to_string(this->focused_entity_ndx); }
		<< "\n"
//		<< "\n  R: " << ftos(&this->player_entity().r) //!!#29: &(world().CFG_GLOBE_RADIUS) // OK now, probably since c365c899
		<< "\n  lifetime: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return this->entity(this->focused_entity_ndx).lifetime == World::Body::Unlimited ?
		                                 "(infinite)" : to_string(this->entity(this->focused_entity_ndx).lifetime); }
		<< "\n  R: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return to_string(this->entity(this->focused_entity_ndx).r); }
		<< "\n  T: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return to_string(this->entity(this->focused_entity_ndx).T); }
//		<< "\n  M: " << ftos(&this->player_entity().mass)
		<< "\n  M: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else { auto M = this->entity(this->focused_entity_ndx).mass / 6e24f;
		                            return ftos(&M)(); } } << " x Earth"
//		<< "\n  x: " << ftos(&this->player_entity().p.x)
//		<<   ", y: " << ftos(&this->player_entity().p.y)
//		<< "\n  vx: " << ftos(&this->player_entity().v.x)
//		<<   ", vy: " << ftos(&this->player_entity().v.y)
		<< "\n  x: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return (ftos(&this->entity(this->focused_entity_ndx).p.x))(); }
		<<   ", y: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return (ftos(&this->entity(this->focused_entity_ndx).p.y))(); }
		<< "\n  vx: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return (ftos(&this->entity(this->focused_entity_ndx).v.x))(); }
		<<   ", vy: " << [&]{ if (this->focused_entity_ndx >= this->entity_count()) return ""s;
		                     else return (ftos(&this->entity(this->focused_entity_ndx).v.y))(); }
	;
  }

	//------------------------------------------------------------------------
	// Debug
#ifdef DEBUG
	auto& debug_hud = ui_gebi(Debug);

//!!Should be Rejected compile-time (with a static_assert):
//!! - well, rejected indeed, but only "fortunately", and NOT by my static_assert!... :-/
//!!	debug_hud << "DBG>" << string("debug");
	static const auto const_debug = "CONST STRING "s; debug_hud << "DBG>" << &const_debug;
//!!shouldn't compile:	debug_hud << "DBG>" << const_debug;
	static       auto debug = "STR "s;                debug_hud << "DBG>" << &debug;
//!!shouldn't compile:	debug_hud << "DBG>" << debug;
	debug_hud << "\nHŐTŰRŐ lótúró [αβ°C]" << "\n";
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
#endif

	//------------------------------------------------------------------------
	// Help
	auto& help_hud = ui_gebi(HelpPanel);
	help_hud
		<< "------------- Actions:\n"
		<< "← → ↑ ↓       Thrust\n"
		<< "SPACE         \"Chemtrail\" sprinkle\n"
		<< "INS           Spawn object(s), +CTRL: 10, +SHIFT: 100\n"
		<< "DEL           Remove object(s), +CTRL: 10, +SHIFT: 100\n"
		<< "------------- God Mode - Metaphysics:\n"
		<< "TAB           Toggle object interactions\n"
		<< "G             Gravity mode\n"
	//	<< "F             Decrease friction, +SHIFT: increase\n"
        //	<< "C             chg. collision mode: pass/stick/bounce\n"
		<< "------------- God Mode - Time Control:\n"
		<< "PAUSE, H      Halt time (model time only, sorry)\n"
		<< "ENTER         Step 1 time slice forward\n"
		<< "BACKSPACE     Step 1 time slice backward\n"
		<< "R             Reverse time (not 100% exact even with fix Δt!)\n" // #376...
		<< "T             Time speedup (half resol.), +SHIFT: slowdn.\n"
		<< "X             Toggle fixed Δt for model updates\n"
		<< "------------- View:\n"
		<< "A W S D       Pan\n"
		<< "MOUSE WHEEL,\n"
		<< "NUMPAD +/-    Zoom\n"
		<< "SHIFT         Auto-scroll to follow player (or other obj.)\n"
		<< "SCROLL LOCK   Toggle locked auto-scroll\n"
		<< "LEFT MOUSE B. Set POI: obj. to follow, or just zoom center\n"
		<< "              +SHIFT: bring player to mouse and/or follow obj.\n"
		<< "HOME          Home in on (center) the player\n"
		<< "SHIFT+HOME    Center (also non-player) followed object\n"
		<< "CTRL+HOME     Reset view (to Home position & default zoom)\n"
		<< "CTRL+ALT      Leave temp. trails (to trace trajectories)\n"
		<< "F11           Toggle fullscreen\n"
		<< "F12           Toggle HUD overlays\n"
		<< "------------- Admin:\n"
		<< "F1-F8         Quicksave (overwrites!), +SHIFT: qickload\n"
		<< "M             Mute/unmute music, N: sound fx\n"
		<< "SHIFT+M       Mute/unmute all audio\n"
		<< "SHIFT+P       Performance (FPS) throttling on/off\n"
		<< "\n"
		<< "ESC           Quit\n"
		<< "\n"
		<< "Command-line options: " << args.exename() << " /?"
	;
//cerr << help_hud;

	help_hud.active(cfg.get("show_help_on_start", true));
#endif
}

//----------------------------------------------------------------------------
void OONApp::onResize(unsigned width, unsigned height) //override
//!!Sink this into the UI!
{
//cerr << "onResize...\n"; //!!TBD: Not called on init; questionable
#ifndef DISABLE_HUD
	ui_gebi(TimingStats).onResize(width, height);
	ui_gebi(WorldData)  .onResize(width, height);
	ui_gebi(ViewData)   .onResize(width, height);
	ui_gebi(ObjectData) .onResize(width, height);
	ui_gebi(HelpPanel)  .onResize(width, height);
	ui_gebi(Debug)      .onResize(width, height);
#endif
	gui.setPosition(4, backend.hci.window().height - gui.getSize().y - 4);
}



//----------------------------------------------------------------------------
unsigned OONApp::add_player(World::Body&& obj) //override
{
	// These are the player modelling differences:
	obj.add_thrusters();
	obj.superpower.gravity_immunity = appcfg.get("sim/player_antigravity", true);
	obj.superpower.free_color = true;
	obj/*.superpower*/.lifetime = World::Body::Unlimited; //!!?? Should this be a superpower instead?

	return (unsigned) //!! Blatant narrowing conv., hoping entity_count() will never overflow `unsigned`...
		add_body(std::forward<World::Body>(obj));
}

void OONApp::remove_player(unsigned)
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

void OONApp::center_entity(size_t id)
{
	oon_main_camera().center_to(entity(id).p);
	oon_main_camera().focus_to({0, 0});
//!!??	oon_main_camera().focus_to(entity(id).p);
}

void OONApp::center_player(unsigned player_id)
{
	assert(player_id == 1);
	center_entity(player_entity_ndx(player_id));
}

void OONApp::follow_entity(size_t id)
{
	auto vpos = oon_main_camera().world_to_view_coord(entity(id).p);
	oon_main_camera().pan(vpos - oon_main_camera().focus_offset);
}

void OONApp::follow_player(unsigned player_id)
{
	assert(player_id == 1);
	follow_entity(player_entity_ndx(player_id));
}


void OONApp::zoom_reset(float factor/* = 0*/)
{
	// Can't just call oon_main_camera().zoom(...), because we need to trigger our zoom_hook!
	if (factor) oon_main_camera().cfg.base_scale *= factor;
	zoom(oon_main_camera().cfg.base_scale / oon_main_camera().scale());
}

void OONApp::zoom(float factor)
{
	//!!pre_zoom_hook(factor);
	oon_main_camera().zoom(factor);
	resize_shapes(factor);
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
void OON_sfml::_adjust_pan_after_zoom(float factor)
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
		|| sfw::getWidget<sfw::CheckBox>("  - forced follow")->get();
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
void OONApp::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
//	if (!obj1->is_player())
//		obj1->color += 0x3363c3;
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
size_t OONApp::add_body(World::Body&& obj) //virtual
// Add new entity (moved) from a template (temporary) obj.
{
	return world().add_body(std::forward<decltype(obj)>(obj));
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
	return add_body({
		.p = { (rand() * p_range) / RAND_MAX - p_range/2 + base.p.x,
		       (rand() * p_range) / RAND_MAX - p_range/2 + base.p.y },
		.v = { (rand() * v_range) / RAND_MAX - v_range/2 + base.v.x * 0.05f,
		       (rand() * v_range) / RAND_MAX - v_range/2 + base.v.y * 0.05f },
		.color = 0xffffff & ((uint32_t) rand() * rand()),
		.mass = M_min + (M_max - M_min) * float(rand())/RAND_MAX,
	});
}

//----------------------------------------------------------------------------
void OONApp::remove_body(size_t ndx) //virtual
{
	world().remove_body(ndx);

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
	remove_body(ndx);
}

//----------------------------------------------------------------------------
void OONApp::spawn(size_t parent_ndx, size_t n)
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
		newborn.lifetime = World::Body::Unlimited;
		newborn.T = parent.T; // #155: Inherit temperature
		newborn.v = parent.v; // 1e5e8be3: Inherit speed
	}
}


//----------------------------------------------------------------------------
//!! Move to SimApp!
void OONApp::_emit_particles(const EmitterConfig& ecfg, size_t emitter_ndx, size_t n)
{
	auto& emitter = entity(emitter_ndx); // Not const: will deplete!

//if (!ecfg.create_mass) cerr <<"DBG> emitter.mass BEFORE burst: "<< emitter.mass <<'\n';

	float p_range = emitter.r * ecfg.position_divergence;
	auto  p_offset = ecfg.eject_offset;
	p_offset += // Also add a portion proportional to the emitter's velocity:
		(emitter.v == Math::Vector2f()) ? //! SFML-DEBUG asserts this, so must be prevented... :-/
			  Math::Vector2f()
			: emitter.v.normalized() * emitter.r * ecfg.offset_factor;
		//!! Also needs a non-linearity (decoupling) factor so higher v can affect it less!

	float v_range = Model::World::CFG_GLOBE_RADIUS * ecfg.velocity_divergence; //!! ...by magic, right? :-/

	float emitter_old_r = emitter.r;

	for (int i = 0; i++ < n;) {
		auto particle_mass = ecfg.particle_mass_min + (ecfg.particle_mass_max - ecfg.particle_mass_min) * float(rand())/RAND_MAX;

		if (!ecfg.create_mass && emitter.mass < particle_mass) {
//cerr << "- Not enough mass to emit particle!\n";
			continue;
		}
//cerr <<"DBG> density: "<< ecfg.particle_density <<'\n';
//cerr <<"DBG>   ==?  : "<< Model::Physics::DENSITY_ROCK * 0.0000000123f <<'\n';

		[[maybe_unused]] auto pndx =
		add_body({
			.lifetime = ecfg.particle_lifetime,
			.density = ecfg.particle_density,
			//!!...Jesus, those "hamfixted" pseudo Δt "factors" here! :-o :)
			.p = { (rand() * p_range) / RAND_MAX - p_range/2 + emitter.p.x + p_offset.x,
			       (rand() * p_range) / RAND_MAX - p_range/2 + emitter.p.y + p_offset.y },
			.v = { (rand() * v_range) / RAND_MAX - v_range/2 + emitter.v.x * ecfg.v_factor + ecfg.eject_velocity.x,
			       (rand() * v_range) / RAND_MAX - v_range/2 + emitter.v.y * ecfg.v_factor + ecfg.eject_velocity.y },
				// Can't just do `...} + ecfg.eject_velocity` above, because C++
			.color = ecfg.color,
			.mass = particle_mass,
		});
//cerr <<"DBG> particle.r: "<< entity(pndx).r <<'\n';

//cerr <<"DBG> emitter v:  "<< emitter.v.x <<", "<< emitter.v.y <<'\n';
//cerr <<"     - eject Δv: "<< ecfg.eject_velocity.x <<", "<< ecfg.eject_velocity.y <<'\n';
//cerr <<"     - part. v:  "<< entity(pndx).v.x <<", "<< entity(entity_count()-1).v.y <<'\n';

		if (!ecfg.create_mass) {
			emitter.mass -= particle_mass;
//cerr <<"DBG> Decreasing emitter.mass by: "<< particle_mass <<'\n';
		}
	}

	if (!ecfg.create_mass) {
		assert(emitter.mass >= 0);
//cerr <<"DBG> emitter.r before recalc: "<< emitter.r <<'\n';
		emitter.recalc();
//cerr <<"DBG> emitter.r after recalc: "<< emitter.r <<'\n';
		resize_shape(emitter_ndx, emitter.r/emitter_old_r);
//cerr <<"DBG> emitter.mass AFTER burst: "<< emitter.mass <<'\n';
	}
}


//----------------------------------------------------------------------------
//!! An exhaust jet should be created for each thruster!
void OONApp::exhaust_burst(size_t base_ndx/* = 0*/, /*Math::Vector2f thrust_vector,*/ size_t n/* = ...*/)
{
	static size_t   add_particles = appcfg.get("sim/exhaust_particles_add", 0);
	static float    exhaust_density = Model::Physics::DENSITY_ROCK * appcfg.get("sim/exhaust_density_ratio", 0.001f);
	static uint32_t exhaust_color = appcfg.get("sim/exhaust_color", 0xaaaaaa);
	static float r_min = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/exhaust_particle_min_size_ratio", 0.02f);
	static float r_max = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/exhaust_particle_max_size_ratio", 0.01f);
	static float airgap = appcfg.get("sim/exhaust_gap", 2.f);

//cerr <<"DBG> cfg.exhaust_density_ratio: "<< appcfg.get("sim/exhaust_density_ratio", 0.001f) <<'\n';
//cerr <<"DBG> -> exhaust_density: "<< exhaust_density <<'\n';

	static EmitterConfig thrust_exhaust_emitter =
	{
		.eject_velocity = {0, 0},
		.v_factor = appcfg.exhaust_v_factor, //!! Should just be calculated instead!,
		.offset_factor = appcfg.exhaust_offset_factor, //!! Should just be calculated instead!
		.particle_lifetime = appcfg.exhaust_lifetime,
		.create_mass = appcfg.get("sim/exhaust_creates_mass", true),
		.particle_density = exhaust_density,
		.position_divergence = appcfg.get("sim/exhaust_divergence", 1.f), // Relative to the emitter radius
		.velocity_divergence = 1.f, //!! Just an exp. "randomness factor" for now!...
		.particle_mass_min = Model::Physics::mass_from_radius_and_density(r_min, Model::Physics::DENSITY_OF_EARTH), //!! WAS: exhaust_density
		.particle_mass_max = Model::Physics::mass_from_radius_and_density(r_max, Model::Physics::DENSITY_OF_EARTH), //!! WAS: exhaust_density
		.color = exhaust_color,
	};

	auto& base = entity(base_ndx); // Not const: will deplete!

// This accidentally creates a lovely rainbow color pattern in the plumes!... :-o
	constexpr const float color_spread = (float)0x111111;
	thrust_exhaust_emitter.color = uint32_t(exhaust_color + color_spread - 2 * color_spread * float(rand())/RAND_MAX);

	//!! This should be calculated from player_thrust_force (around 3e36 N curerently)!
	const auto eject_v = 4e9f;// * abs(appcfg.exhaust_v_factor/2); //! Since this is a property of the thrusters, don't
	                                                               //! adjust with the full v-factor!... Just a hint! :)
	if (base.thrust_up.thrust_level()) {
		thrust_exhaust_emitter.eject_velocity = {0, -eject_v};
		thrust_exhaust_emitter.eject_offset = {0, -base.r * airgap};
		_emit_particles(thrust_exhaust_emitter, base_ndx, add_particles ? add_particles : n);
	}
	if (base.thrust_down.thrust_level()) {
		thrust_exhaust_emitter.eject_velocity = {0, eject_v};
		thrust_exhaust_emitter.eject_offset = {0, base.r * airgap};
		_emit_particles(thrust_exhaust_emitter, base_ndx, add_particles ? add_particles : n);
	}
	if (base.thrust_left.thrust_level()) {
		thrust_exhaust_emitter.eject_velocity = {eject_v, 0};
		thrust_exhaust_emitter.eject_offset = {base.r * airgap, 0};
		_emit_particles(thrust_exhaust_emitter, base_ndx, add_particles ? add_particles : n);
	}
	if (base.thrust_right.thrust_level()) {
		thrust_exhaust_emitter.eject_velocity = {-eject_v, 0};
		thrust_exhaust_emitter.eject_offset = {-base.r * airgap, 0};
		_emit_particles(thrust_exhaust_emitter, base_ndx, add_particles ? add_particles : n);
	}
}


//----------------------------------------------------------------------------
void OONApp::shield_energize(size_t emitter_ndx, /*Math::Vector2f shoot_vector,*/ size_t n/* = ...*/)
{
	static float    particle_density = Model::Physics::DENSITY_ROCK * appcfg.get("sim/shield_density_ratio", 0.001f);
	static uint32_t color = appcfg.get("sim/shield_color", 0xffff99);
	static float r_min = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/shield_particle_min_size_ratio", 0.02f);
	static float r_max = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/shield_particle_max_size_ratio", 0.01f);
//cerr <<"DBG> cfg.exhaust_density_ratio: "<< appcfg.get("sim/exhaust_density_ratio", 0.001f) <<'\n';
//cerr <<"DBG> -> shield_density: "<< particle_density <<'\n';
	static EmitterConfig emitter_cfg =
	{
		.eject_velocity = {0, 0},
		.v_factor = appcfg.get("sim/shield_v_factor", 0.1f),
		.offset_factor = appcfg.get("sim/shield_offset_factor", 4.f),
		.particle_lifetime = appcfg.get("sim/shield_decay_time", 5.f),
		.create_mass = false, // Disabled: appcfg.get("sim/shield_creates_mass", false),
		.particle_density = particle_density,
		.position_divergence = appcfg.get("sim/shield_initial_spread", 10.f), //!! Just an exp. "randomness factor" for now!... Relative to emitter radius.
		.velocity_divergence = appcfg.get("sim/shield_divergence", 1.f), //!! Just an exp. "randomness factor" for now!...
		.particle_mass_min = Model::Physics::mass_from_radius_and_density(r_min, Model::Physics::DENSITY_OF_EARTH),
		.particle_mass_max = Model::Physics::mass_from_radius_and_density(r_max, Model::Physics::DENSITY_OF_EARTH),
		.color = color,
	};

//	emitter_cfg.eject_velocity = entity(emitter_ndx).v;
	_emit_particles(emitter_cfg, emitter_ndx, n ? n : appcfg.shield_burst_particles);
}


//----------------------------------------------------------------------------
void OONApp::chemtrail_burst(size_t emitter_ndx/* = 0*/, size_t n/* = ...*/)
{
	static float chemtrail_v_factor      = appcfg.get("sim/chemtrail_v_factor", 0.1f);
	static float chemtrail_offset_factor = appcfg.get("sim/chemtrail_offset_factor", 0.2f);
	static float chemtrail_lifetime      = appcfg.get("sim/chemtrail_lifetime", Model::World::Body::Unlimited);
	static bool  chemtrail_creates_mass  = appcfg.get("sim/chemtrail_creates_mass", true);
	static float chemtrail_density       = Model::Physics::DENSITY_ROCK * appcfg.get("sim/chemtrail_density_ratio", 0.001f);
	static float chemtrail_divergence    = appcfg.get("sim/chemtrail_divergence", 1.f);
	static float r_min = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/chemtrail_particle_min_size_ratio", 0.02f);
	static float r_max = Model::World::CFG_GLOBE_RADIUS * appcfg.get("sim/chemtrail_particle_max_size_ratio", 0.1f);
	static float M_min = Model::Physics::mass_from_radius_and_density(r_min, chemtrail_density);
	static float M_max = Model::Physics::mass_from_radius_and_density(r_max, chemtrail_density);

	auto& emitter = entity(emitter_ndx); // Not const: will deplete!
	float p_range = emitter.r * 5;
	float v_range = Model::World::CFG_GLOBE_RADIUS * chemtrail_divergence; //!! ...by magic, right? :-/

	float emitter_old_r = emitter.r;

	for (int i = 0; i++ < n;) {
		auto particle_mass = M_min + (M_max - M_min) * float(rand())/RAND_MAX;
		if (!chemtrail_creates_mass && emitter.mass < particle_mass) {
//cerr << "- Not enough mass to emit particle...\n";
			continue;
		}

		add_body({
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
	resize_shape(emitter_ndx, emitter.r / emitter_old_r);
}

} // namespace OON