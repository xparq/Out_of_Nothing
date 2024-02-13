#include "OON.hpp"

#include "Engine/Backend/HCI.hpp"
#include "sfw/GUI.hpp" //!! Used to be in OONApp only, but since scroll_locked() requires it...
                       //!! (And sooner or later it must be usable unrestricted anyway!
#include "UI/hud.hpp"  //!! <-- And also this would be integrated there, too, eventually.
                       //!! And we're already using keystate() here, too, shamelessly! ;) )
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>
#include "sz/debug.hh"

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


//! This still needs to be a double nested lambda: the outer wrapper "converts" the
//! signature to match the string-returning nullary functions the HUD stuff supports.
static auto ftos = [](auto* ptr_x) { return [ptr_x]() { static constexpr size_t LEN = 15;
		char buf[LEN + 1]; auto [ptr, ec] = std::to_chars(buf, buf+LEN, *ptr_x);
		return string(ec != std::errc() ? "???" : (*ptr = 0, buf));
	};
};


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

	// General/performance controls...
	auto	perf_form = gui_main_hbox->add(new Form);
		perf_form->add("FPS", new ProgressBar({.length=60, .range={0, 120}, .clamp=false}));
		perf_form->add("- limit", new Slider({.length=60, .range={0, 120}, .step=30}))
			->set(fps_throttling())
			->setCallback([this](auto* w){fps_throttling((unsigned)w->get());});
		perf_form->add("Fixed model Δt", new CheckBox(
			[&](auto*){ this->toggle_fixed_model_dt(); }, cfg.fixed_model_dt_enabled));

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// UI/View controls...
	auto	view_form = gui_main_hbox->add(new Form);
		view_form->add("Show HUDs", new CheckBox([&](auto*){ this->toggle_huds(); }, huds_active()));
		    gui.recall("Show HUDs")->setTooltip("Press [?] to toggle the Help panel");
		view_form->add("Grid lines", new CheckBox([&](auto* w){oon_main_camera().cfg.gridlines = w->get();}, oon_main_camera().cfg.gridlines));
		view_form->add("Pan follows object", new CheckBox)->disable(); // Will be updated by the ctrl. loop!
		view_form->add("  - forced follow", new CheckBox); // Will be polled by the control loop!

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// Audio...
	auto left_vbox = gui_main_hbox->add(new VBox);
		auto	volrect = left_vbox->add(new Form, "VolForm");
			volrect->add("Volume", new Slider({.length=70, /*.orientation=Vertical*/})) //!! not needed: , "volume slider")
			->setCallback([&](auto* w){backend.audio.volume(w->get());})
			->update(75); // %
		auto	audio_onoff = left_vbox->add(new Form, "AudioOnOffForm");
			audio_onoff->add("Audio: ", new CheckBox([&](auto*){backend.audio.toggle_audio();}, backend.audio.enabled));
			audio_onoff->add(" - FX: ", new CheckBox([&](auto*){backend.audio.toggle_sounds();}, backend.audio.fx_enabled));

	gui_main_hbox->add(new Label(" ")); // Just a vert. spacer

	// Physics - exp. tweaks...
	auto	phys_form = gui_main_hbox->add(new Form);
		auto g_select = new OptionsBox<World::GravityMode>();
			g_select->add("Off",          World::GravityMode::Off);
			g_select->add("Hyperbolic",   World::GravityMode::Hyperbolic);
			g_select->add("Realistic",    World::GravityMode::Realistic);
			g_select->add("Experimental", World::GravityMode::Experimental);
			g_select->set(World::GravityMode::Default);
			g_select->setCallback([&](auto* w){ this->world().gravity_mode = w->get(); });
		phys_form->add("Gravity mode", g_select)
			->set(world().gravity_mode);
		phys_form->add(" - bias", new sfw::Slider({.length=80, .range={-3.0, 3.0}, .step=0}))
			->setCallback([&](auto* w){ this->world().gravity = Phys::G //!! <- NO! Either use the original base val, or just modify the current .gravity!
				* Math::power(10.f, w->get()); })
			->set(0);
#ifndef DISABLE_FULL_INTERACTION_LOOP
		phys_form->add("Full int. loop", new sfw::CheckBox([&](auto* w){ this->world().loop_mode = w->get() ? World::LoopMode::Full : World::LoopMode::Half; },
				world().loop_mode == World::LoopMode::Full));
#endif
		phys_form->add("Friction", new sfw::Slider({.length=80, .range={-1.0, 1.0}, .step=0}))
			->setCallback([&](auto* w){ this->world().friction = w->get(); })
			->set(world().friction);

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// Save/load...
	auto	saveload_form = gui_main_hbox->add(new Form);
		saveload_form->add("File", new TextBox);
		auto	saveload_buttons = saveload_form->add("", new HBox);
			saveload_buttons->add(new Button("Save"))
				->setTextColor(sf::Color::Black)->setColor(sfw::Color("#f002"))
				->setCallback([&]{
					if (auto* fname_widget = (TextBox*)gui.recall("File"); fname_widget) {
						auto fname = fname_widget->get();
						bool compress = cfg.save_compressed;
						if (auto* compress_widget = (CheckBox*)gui.recall("Compress"); compress_widget)
							compress = compress_widget->get();
						this->save_snapshot(fname.empty() ? "UNTITLED.save" : fname.c_str(),
							compress ? SaveOpt::Compress : SaveOpt::Raw);
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
		//!! Basically for testing only:
		saveload_form->add("Compress", new CheckBox(cfg.save_compressed));

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// Only position after built, so it has its size:
	//!! This is also done in onResize(), but that can't be invoked on init (#462) until #515, so...:
	gui.setPosition(4, main_window_height() - gui.getSize().y - 4);
		//!! For that 4 above: sfw is still too lame for styling margins/padding... :-/
		//!! Also, negative coords. aren't special in SFW, so this just goes off-screen: gui.setPosition({100, -200});

#ifndef DISABLE_HUD
	_setup_HUDs();
#endif
}

#ifndef DISABLE_HUD
void OONApp::_setup_HUDs()
{
	//!!?? Why do all these member pointers just work, also without so much as a warning,
	//!!?? in this generic pointer passing context?!
	//!!
	//!! "Evenfurthermore": why do all these insane `this` captures apparently survive
	//!! all the obj recreation shenanigans (they *are* recreated, right??...) after
	//!! a World reload?!?!?!
	//!!

	//------------------------------------------------------------------------
	// Timing
	ui_gebi(TimingStats)
		<< "FPS: " << [this](){ return to_string(1 / (float)avg_frame_delay); }
			<< [this](){ return fps_throttling() ? " (fixed)" : ""; }
		<< "\nlast frame Δt: " << [this](){ return to_string(time.last_frame_delay * 1000.0f) + " ms"; }
		<< "\nmodel Δt: " << [this](){ return to_string(time.last_model_Δt * 1000.0f) + " ms"; }
		<<            " " << [this](){ return cfg.fixed_model_dt_enabled ? "(fixed)" : ""; }
		<< "\ncycle: " << [this](){ return to_string(iterations); }
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
		<< "\n    avg.: " << [this]{ return to_string(time.model_Δt_stats.average());}
	;
//cerr << timing_hud;

	//------------------------------------------------------------------------
	// World
	ui_gebi(WorldData)
		<< "# of objs.: " << [this](){ return to_string(entity_count()); }
		<< "\nBody interactions: " << &const_world()._interact_all
		<< "\nGravity mode: " << [this](){ return to_string((unsigned)const_world().gravity_mode); }
		<< "\n  - strength: " << &const_world().gravity
		<< "\nDrag: " << ftos(&this->const_world().friction)
		<< "\n"
	;

	//------------------------------------------------------------------------
	// View
	ui_gebi(ViewData)
		<< "MAIN CAMERA:"
		<< "\n  X: " << &oon_main_camera().offset.x << ", Y: " << &oon_main_camera().offset.y
		//!! to_string() fucked it up and returned "0.000000" for e.g. 0.00000005f! :-o (#509)
		//!! << "\n  Scale: " << [this](){ return to_string(oon_main_camera().scale() * 1e6f); } << " x 1e-6"
		<< "\n  Base scale: " << &oon_main_camera().cfg.base_scale
		<< "\n  Zoom adj.: "<< [this](){ return to_string(oon_main_camera().scale() / oon_main_camera().cfg.base_scale); }
		<< "\n  Focus pt.: "<< &oon_main_camera().focus_offset.x << ", " << &oon_main_camera().focus_offset.y
/*
		<< "\nVIEWPORT:"
		<< "\n_edge_x_min: "<< &oon_main_camera()._edge_x_min
		<< "\n_edge_x_min: "<< &oon_main_camera()._edge_x_max
		<< "\n_edge_y_min: "<< &oon_main_camera()._edge_y_min
		<< "\n_edge_y_min: "<< &oon_main_camera()._edge_y_max
*/	;

//	???_hud << "\nPress ? for help...";

	//------------------------------------------------------------------------
	// "Object Observer"
	if (!(player_entity_ndx() < entity_count())) {
		cerr << "- INTERNAL ERROR: UI/PlayerHUD init before player entity init!\n";
	} else {
		_setup_HUD_ObjMonitor();
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
		<< "LEFT ALT      Shield particles (doing nothing yet...)\n"
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
		<< "T             Time speedup (half resol.), +SHIFT: slowdown\n"
		<< "X             Toggle fixed Δt for model updates\n"
		<< "------------- View:\n"
		<< "A W S D       Pan\n"
		<< "MOUSE WHEEL,\n"
		<< "NUMPAD +/-    Zoom\n"
		<< "SHIFT         Auto-scroll to follow player (or other obj.)\n"
		<< "SCROLL LOCK   Toggle locked auto-scroll (to follow obj.)\n"
		<< "MOUSE CLICK   Set focus point (zoom center) and/or select obj.\n"
		<< "SHIFT+CLICK   - also reposition the view & lock scrolling\n"
		<< "HOME          Home in on (center) the player\n"
		<< "SHIFT+HOME    - also lock scrolling\n"
		<< "CTRL+HOME     Reset view to Home position (keep the zoom)\n"
		<< "NUMPAD 5      Reset view to Home position & default zoom\n"
		<< "L.CTRL+R.CTRL Leave trails (by not clearing the screen)\n"
		<< "MOUSE MOVE    Show data of hovered object in the obj. HUD\n"
		<< "MOUSE DRAG    Pan freely (untracking any prev. focus obj.)\n"
		<< "SHIFT+MOVE    Pan with the focus pt./obj locked to the mouse\n"
		<< "F11           Toggle fullscreen\n"
		<< "F12           Toggle (most) HUD overlays\n"
		<< "------------- Admin:\n"
		<< "F1-F8         Quicksave (overwrites!), +SHIFT: qickload\n"
		<< "M             Mute/unmute music, N: sound fx\n"
		<< "SHIFT+M       Mute/unmute all audio\n"
//!! #543	<< "SHIFT+P       Performance (FPS) throttling on/off\n"
		<< "RIGHT ALT     Stream debug info to the terminal\n"
		<< "\n"
		<< "ESC           Quit\n"
		<< "\n"
		<< "Command-line options: " << args.exename() << " /?"
	;
//cerr << help_hud;

	help_hud.active(cfg.get("show_help_on_start", true));
}


//----------------------------------------------------------------------------
void OONApp::_setup_HUD_ObjMonitor(/*!!, mode/config...!!*/)
//! REMEMBER:
//!	This is just a one-time setup function!
//!	All the HUD can do later, as the bound data changes, is to display
//!	that updated values, possibly via lambdas/closures created here.
//!	So... ALL LOGIC MUST LIVE INSIDE THOSE!
//!
//!	Anything more complex than that MUST BE DONE OUTSIDE, in the app,
//!	by reinitializing the entire HUD accordingly!
//!	E.g., a HUD reconfig should be triggered:
//!	- if another object has to be followed (for whatever reason)
//!	  - Well, not with this current HUD setup, which can cover any of them (even null) implicitly!
//!	    But a simplified (faster) version should drop all that bloat and rely on reconfig.!
//!	- or the object being tracked dies (!! Non-trivial to hook back right up to this OONApp
//!	  level to follow-up with a HUD reset! The current one is safeguerded against invalid IDs instead!)
{
	//! Setting up player data watches below requires that the player
	//! entity have actually been created already!
	//! If not, an assertion will fail, but only in the DEBUG build!
	//! So... Just checking it run-time, too, as I have made this mistake
	//! too many times now... :)
  	if ( !(player_entity_ndx() < entity_count()) ) {
		cerr << "- INTERNAL ERROR: UI/PlayerHUD init before player entity init!\n";
		return;
  	}

	static auto no_obj = [this]() { //! Either static, or [no_obj, obj] for the callers (instead of just [&])!...
//cerr << "no_obj - hovered_entity_ndx: " << this->hovered_entity_ndx << "\n";
//		return hovered_entity_ndx != ~0u ? hovered_entity_ndx >= entity_count()
//		                                 : focused_entity_ndx >= entity_count();
		return !( hovered_entity_ndx < entity_count() ||
		          focused_entity_ndx < entity_count() );
	};
	static auto obj = [this]() -> const Entity& {
		return entity(hovered_entity_ndx != ~0u ? hovered_entity_ndx : focused_entity_ndx);
	};
	static auto id = [this]() -> size_t {
		return hovered_entity_ndx != ~0u ? hovered_entity_ndx : focused_entity_ndx;
	};

	ui_gebi(ObjMonitor)
		<< [&]{ if (no_obj()) return "<NOTHING>"s;
			if (id() >= entity_count()) return "INVALID ENTITY #"s + to_string(id());
			if (id() == player_entity_ndx()) return "Player #"s + to_string(player_entity_ndx());
			else                             return "Object #"s + to_string(id()); }
		<< "\n"
//		<< "\n  R: " << ftos(&this->player_entity().r) //!!#29: &(world().CFG_GLOBE_RADIUS) // OK now, probably since c365c899
		<< "\n  lifetime: " << [&]{ return no_obj() ? ""s :
		                       obj().lifetime == Entity::Unlimited ? "(infinite)" : to_string(obj().lifetime); }
		<< "\n  R: " << [&]{ return no_obj() ? ""s : to_string(obj().r); }
		<< "\n  T: " << [&]{ return no_obj() ? ""s : to_string(obj().T); }
//		<< "\n  M: " << ftos(&this->player_entity().mass)
		<< "\n  M: " << [&]{ if (no_obj()) { return ""s; }
		                     else { auto M = obj().mass / 6e24f; return ftos(&M)(); } }
		             << " x Earth"
//		<< "\n  x: " << ftos(&this->player_entity().p.x)
//		<<   ", y: " << ftos(&this->player_entity().p.y)
//		<< "\n  vx: " << ftos(&this->player_entity().v.x)
//		<<   ", vy: " << ftos(&this->player_entity().v.y)
		<< "\n  x: "  << [&]{ return no_obj() ? ""s : (ftos(&obj().p.x))(); }
		<<   ", y: "  << [&]{ return no_obj() ? ""s : (ftos(&obj().p.y))(); }
		<< "\n  vx: " << [&]{ return no_obj() ? ""s : (ftos(&obj().v.x))(); }
		<<   ", vy: " << [&]{ return no_obj() ? ""s : (ftos(&obj().v.y))(); }
	;
}

#endif // DISABLE_HUD


//----------------------------------------------------------------------------
void OONApp::onResize(unsigned width, unsigned height) //override
//!!Sink this into the UI!
{
//cerr << "onResize...\n"; //!!TBD: Not called on init; questionable
#ifndef DISABLE_HUD
	ui_gebi(TimingStats).onResize(width, height);
	ui_gebi(WorldData)  .onResize(width, height);
	ui_gebi(ViewData)   .onResize(width, height);
	ui_gebi(ObjMonitor) .onResize(width, height);
	ui_gebi(HelpPanel)  .onResize(width, height);
	ui_gebi(Debug)      .onResize(width, height);
#endif
	gui.setPosition(4, main_window_height() - gui.getSize().y - 4);
}

} // namespace OON
