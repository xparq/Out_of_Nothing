#include "OON_UI.hpp"
#include "OON.hpp"

#include "Engine/UI/hud.hpp"  //!! <-- And also this would be integrated there, too, eventually.
                       //!! And we're already using keystate() here, too, shamelessly! ;) )
#include "Engine/Backend/HCI.hpp"

#include "Engine/diag/Error.hpp"
//#include "Engine/diag/Log.hpp"
//#include "sz/DBG.hh"

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


//! This still needs to be a double nested lambda: the outer wrapper "converts the
//! signature" to match the string-returning nullary functions the HUD stuff supports.
static auto ftos = [](auto* ptr_x) { return [ptr_x]() { static constexpr size_t LEN = 25; // max length of double
		char buf[LEN + 1]; auto [ptr, ec] = std::to_chars(buf, buf+LEN, *ptr_x);
		return string(ec != std::errc() ? "<ERR>" : (*ptr = 0, buf));
	};
};


//----------------------------------------------------------------------------
bool OONApp::show_cmdline_help(const Args& args, const char* banner)
{
	banner = "\"Out of Nothing\" - Experimental N-body simulation toy\n";

	SimApp::show_cmdline_help(args, banner);

	//!! Evn this "custom options" part should be automated some day:
	cout << R"(
  -C cfgfile
          Load configuration from 'cfgfile'. If not found, abort.
	  If omitted, ./default.cfg is tried, and if that doesn't exist,
	  internal hardcoded defaults will be used as a fallback.

  ...lots more (yet to be documented here, sorry)!
)";

	return false;
}


//----------------------------------------------------------------------------
#ifndef DISABLE_HUDS
void OONApp::toggle_huds()  { _ui_show_huds = !_ui_show_huds; }
bool OONApp::huds_active()  { return _ui_show_huds; }
void OONApp::toggle_help()  { ui_gebi(HelpPanel).active(!ui_gebi(HelpPanel).active()); }
#endif

//----------------------------------------------------------------------------
void OONApp::ui_setup()
{
	auto& app = *this; //!! Remnant from an aborted attempt to make this _UI_::setup(SimApp& app)...
	using namespace myco;

	// If the GUI is used as a translucent overlay, an alpha-enabled bgColor
	// must be applied. The clearBackground option must be left at its default (true):
	//Theme::clearBackground = false;
	// Button label and caption color:
	Theme::styling_profile_ref(myco::style::Tactile).textColor = myco::Color("#ee9"); //!!YUCK!!
	auto gui_main_hbox = gui.add(new HBox);

	// General/performance controls...
	auto	perf_form = gui_main_hbox->add(new Form);
		perf_form->add("FPS", new ProgressBar({.length=60, .range={0, 120}, .clamp=false}));
		perf_form->add("- limit", new Slider({.length=60, .range={0, 120}, .step=30}))
			->set(app.fps_throttling())
			->setCallback([&](auto* w){ app.fps_throttling((unsigned)w->get()); });
		perf_form->add("Fixed model Δt", new CheckBox(
			[&](auto*){ app.toggle_fixed_model_dt(); }, app.cfg.fixed_model_dt_enabled));

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// UI/View controls...
	auto	view_form = gui_main_hbox->add(new Form);
		view_form->add("Show HUDs", new CheckBox([&](auto*){ app.toggle_huds(); }, app.huds_active()));
		    gui.recall("Show HUDs")->set_tooltip("Press [?] to toggle the Help panel");
		view_form->add("Grid lines", new CheckBox([&](auto* w){ app.oon_main_camera().cfg.gridlines = w->get(); },
		                                          app.oon_main_camera().cfg.gridlines));
		view_form->add("Pan follows object", new CheckBox)->disable(); // Will be updated by the ctrl. loop!
		view_form->add("  - forced follow", new CheckBox); // Will be polled by the control loop!

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// Audio...
	auto left_vbox = gui_main_hbox->add(new VBox);
		auto	volrect = left_vbox->add(new Form, "VolForm");
			volrect->add("Volume", new Slider({.length=70/*, .orientation=Vertical*/, .preset=75})) // %
			->setCallback([&](auto* w){ app.backend.audio.volume(w->get()); });
		auto	audio_onoff = left_vbox->add(new Form, "AudioOnOffForm");
			audio_onoff->add("Audio: ", new CheckBox([&](auto*){ app.backend.audio.toggle_audio(); },  app.backend.audio.enabled));
			audio_onoff->add(" - Music: ", new CheckBox([&](auto*){ app.backend.audio.toggle_music(); }, app.backend.audio.music_enabled));
			audio_onoff->add(" - FX: ", new CheckBox([&](auto*){ app.backend.audio.toggle_sounds(); }, app.backend.audio.fx_enabled));

	gui_main_hbox->add(new Label(" ")); // Just a vert. spacer

	// Physics - exp. tweaks...
	auto	phys_form = gui_main_hbox->add(new Form);
		auto g_select = new GravityModeSelector();
			g_select->add("Off",          World::GravityMode::Off);
			g_select->add("Hyperbolic",   World::GravityMode::Hyperbolic);
			g_select->add("Realistic",    World::GravityMode::Realistic);
			g_select->add("Experimental", World::GravityMode::Experimental);
			g_select->set(World::GravityMode::Default);
			g_select->setCallback([&](auto* w){ app.world().gravity_mode = w->get(); });
		phys_form->add("Gravity mode", g_select)
			->set(app.world().gravity_mode);
		phys_form->add(" - bias", new myco::Slider({.length=80, .range={-3.0, 3.0}, .step=0, .preset=0}))
			->setCallback([&](auto* w){ app.world().gravity = Phys::G //!! <- NO! Either use the original base val, or just modify the current .gravity!
				* Math::power(10.f, w->get()); })
			->set(0);
#ifndef DISABLE_FULL_INTERACTION_LOOP
		phys_form->add("Full int. loop", new myco::CheckBox([&](auto* w){ app.world().loop_mode = w->get() ? World::LoopMode::Full : World::LoopMode::Half; },
				app.world().loop_mode == World::LoopMode::Full));
#endif
		phys_form->add("Friction", new myco::Slider({.length=80, .range={-1.0, 1.0}, .step=0, .preset=0}))
			->setCallback([&](auto* w){ app.world().friction = w->get(); })
			->set(app.world().friction);

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// Save/load...
	auto	saveload_form = gui_main_hbox->add(new Form);
		saveload_form->add("File", new TextField);
		auto	saveload_buttons = saveload_form->add("", new HBox);
			saveload_buttons->add(new Button("Save"))
				->setTextColor(sf::Color::Black)->setColor(myco::Color("#f002"))
				->setCallback([&]{
					if (auto* fname_widget = (TextField*)gui.recall("File"); fname_widget) {
						auto fname = fname_widget->get();
						bool compress = app.cfg.save_compressed;
						if (auto* compress_widget = (CheckBox*)gui.recall("Compress"); compress_widget)
							compress = compress_widget->get();
						app.save_snapshot(fname.empty() ? "UNTITLED.save" : fname.c_str(),
							compress ? SaveOpt::Compress : SaveOpt::Raw);
					}
				});
			saveload_buttons->add(new Button("Load"))
				->setTextColor(sf::Color::Black)->setColor(myco::Color("#0f02"))
				->setCallback([&]{
					if (auto* fname_widget = (TextField*)gui.recall("File"); fname_widget) {
						auto fname = fname_widget->get();
						app.load_snapshot(fname.empty() ? "UNTITLED.save" : fname.c_str());
					}
				});
		//!! Basically for testing only:
		saveload_form->add("Compress", new CheckBox(app.cfg.save_compressed));

	gui_main_hbox->add(new Label(" ")); // just a vert. spacer

	// Only position after built, so it has its size:
	//!! This is also done in onResize(), but that can't be invoked on init (#462) until #515, so...:
	gui.setPosition(4, app.main_window_height() - gui.getSize().y() - 4);
		//!! For that 4 above: mycoGUI is still too lame for styling margins/padding... :-/
		//!! Also, negative coords. aren't special in myco, so this just goes off-screen: gui.setPosition({100, -200});

#ifndef DISABLE_HUDS
	ui_setup_HUDs();
#endif
}

#ifndef DISABLE_HUDS
void OONApp::ui_setup_HUDs()
{
	//!!??
	//!!?? Why do all those member pointers in ui_setup_HUD_...() just work — also without
	//!!?? so much as a warning, in those generic pointer-passing contexts?!
	//!!??
	//!!?? "Evenfurthermore": why do all those insane `this` captures apparently survive
	//!!?? all the obj recreation shenanigans (they *are* recreated, right??...) after
	//!!?? a World reload?!?!?!
	//!!??

	//------------------------------------------------------------------------
	ui_setup_HUD_World();
	ui_setup_HUD_Time();
	ui_setup_HUD_View();
	// "Object Observer"...
	if (!(player_entity_ndx() < entity_count())) {
		Bug("UI/PlayerHUD init before player entity init!");
	} else {
		ui_setup_HUD_ObjMonitor();
	}
	// Help...
	ui_setup_HUD_Help();
	auto& help_hud = ui_gebi(HelpPanel);
	help_hud.active(cfg.get("show_help_on_start", true));

	//------------------------------------------------------------------------
	// Debug
//#ifdef DEBUG
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
//		<< "test fn->string: " << hud_test_callback_string << "\n"
//		<< "test fn->ccptr: " << hud_test_callback_ccptr << "\n"
		<< "test λ->ccptr: " << []{ return "autoconv to string?..."; } << "\n"
//!!NOPE:	<< "test λ->int: " << []{ return 0xebba; } << "\n"
//!!NOPE:	<< "test λ->float: " << []{ return 12.345; } << "\n"
		// By-value data that are not closures:
		<< "test val. string: " << "temp"s << "\n"
		<< "test val. int: " << 12345 << "\n"
		<< "test val. float: " << 12.345f << "\n"
		<< "test val. double: " << 1e300 << "\n"
//!!NOT YET:	<< "test val. bool: " << true << "\n"

		<< "UI focused? " << [&]{ return this->gui.focused() ? "YES" : "no"; }

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
//#endif
}


//------------------------------------------------------------------------
void OONApp::ui_setup_HUD_World(/*!!, mode/config...!!*/)
{
	ui_gebi(WorldData)
		<< "# of objs.: " << [this](){ return to_string(entity_count()); }
		<< "\nEntity interactions: " << &const_world()._interact_all
		<< "\nGravity mode: " << [this](){ return to_string((unsigned)const_world().gravity_mode); }
		<< "\n  - strength: " << &const_world().gravity
		<< "\nDrag: " << ftos(&this->const_world().friction)
		<< "\n"
	;
}

//------------------------------------------------------------------------
void OONApp::ui_setup_HUD_Time(/*!!, mode/config...!!*/)
{
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
		<< "\nModel timing stats (s):"
//		<< "\n    updates: " << &time.model_Δt_stats.samples
		<< "\n    total t: " << &time.model_Δt_stats.total
		<< "\n  Δt, as scaled x" << &time.scale << ":"
		<< "\n    last: " << &time.model_Δt_stats.last
		<< "\n    |min|: " << &time.model_Δt_stats.umin
		<< "\n    |max|: " << &time.model_Δt_stats.umax
//		<< "\n    min: " << &time.model_Δt_stats.min
//		<< "\n    max: " << &time.model_Δt_stats.max
		<< "\n    avg.: " << [this]{ return to_string(time.model_Δt_stats.average());}
	;
//cerr << timing_hud;
}

//------------------------------------------------------------------------
void OONApp::ui_setup_HUD_View(/*!!, mode/config...!!*/)
{
	ui_gebi(ViewData)
		<< "MAIN CAMERA:"
		<< "\n  X: " << &oon_main_camera().view_offset.x << ", Y: " << &oon_main_camera().view_offset.y
		//!! to_string() fucked it up and returned "0.000000" for e.g. 0.00000005f! :-o (#509)
		//!! << "\n  Scale: " << [this](){ return to_string(oon_main_camera().scale() * 1e6f); } << " x 1e-6"
		<< "\n  Base scale: " << &oon_main_camera().cfg.base_scale
		<< "\n  Zoom adj.: "<< [this](){ return to_string(oon_main_camera().scale() / oon_main_camera().cfg.base_scale); }
		<< "\n  Focus: "<< &oon_main_camera().focus_offset.x << ", " << &oon_main_camera().focus_offset.y
/*
		<< "\nVIEWPORT:"
		<< "\n_edge_x_min: "<< &oon_main_camera()._edge_x_min
		<< "\n_edge_x_min: "<< &oon_main_camera()._edge_x_max
		<< "\n_edge_y_min: "<< &oon_main_camera()._edge_y_min
		<< "\n_edge_y_min: "<< &oon_main_camera()._edge_y_max
*/	;
}

//----------------------------------------------------------------------------
void OONApp::ui_setup_HUD_ObjMonitor(/*!!, mode/config...!!*/)
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
		Bug("UI/PlayerHUD init before player entity init!");
		return;
  	}

	static auto id = [this]() -> EntityID {
		return        hovered_entity_ndx != Entity::NONE ? hovered_entity_ndx : focused_entity_ndx;
	};
	static auto obj = [this]() -> const Entity& {
		return entity(hovered_entity_ndx != Entity::NONE ? hovered_entity_ndx : focused_entity_ndx);
	};
	static auto no_obj = [this]() { //! Either static, or [no_obj, obj] for the callers (instead of just [&])!...
//cerr << "no_obj - hovered_entity_ndx: " << this->hovered_entity_ndx << "\n";
//		return hovered_entity_ndx != Entity::NONE ? hovered_entity_ndx >= entity_count()
//		                                 : focused_entity_ndx >= entity_count();
		return !( hovered_entity_ndx < entity_count() ||
		          focused_entity_ndx < entity_count() );
	};

	ui_gebi(ObjMonitor)
		<< [&]{ if (no_obj()) return "<NOTHING>"s;
			if (id() >= entity_count()) return "INVALID ENTITY #"s + to_string(id());
			if (id() == player_entity_ndx()) return "Player #"s + to_string(player_entity_ndx() + 1);
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

//------------------------------------------------------------------------
void OONApp::ui_setup_HUD_Help(/*!!, mode/config...!!*/)
{
	auto& help_hud = ui_gebi(HelpPanel);
	help_hud
		<< "-------------- Actions:\n"
		<< "A W S D        Thrust\n"
		<< "SPACE          \"Chemtrail\" sprinkle\n"
		<< "LEFT ALT       Emit shield particles (no actual shielding yet)\n"
		<< "INS            Spawn object(s), +CTRL: 10, +SHIFT: 100\n"
		<< "DEL            Remove object(s), +CTRL: 10, +SHIFT: 100\n"
		<< "-------------- God Mode - Time Control:\n"
		<< "PAUSE, H       Halt time (model time only — sorry! ;) )\n"
		<< "ENTER          Step forward 1 time slice, when paused\n"
		<< "BACKSPACE      Step back 1 time slice, when paused\n"
		<< "R              Reverse time (not 100% exact even with fix Δt!)\n" // #376...
		<< "T              Time warp ↑ (half sim. res.), +SHIFT: ↓ (dbl. res)\n"
		<< "X              Toggle fixed Δt for model updates\n"
		<< "-------------- View:\n"
		<< "← → ↑ ↓        Pan\n"
		<< "MOUSE WHEEL,\n"
		<< "NUMPAD +/-     Zoom\n"
		<< "SHIFT          Auto-scroll to follow the focused object\n"
		<< "SCROLL LOCK    Lock/unlock auto-scroll\n"
		<< "MOUSE CLICK    Set focused object (or any point as zoom center)\n"
		<< "HOME           Home in on (center) the focused obj. (If\n"
		<< "               no focused obj. then select the player.)\n"
		<< "SHIFT+HOME     - also keep auto-scrolling (as usual with SHIFT)\n"
		<< "CTRL+HOME      Reset view to Home position (keep the zoom)\n"
		<< "NUMPAD 5       Reset view to Home position & default zoom\n"
		<< "L.CTRL+R.CTRL  Leave trails (by not clearing the screen)\n"
		<< "MOUSE MOVE     Show data of hovered object in the obj. HUD\n"
		<< "MOUSE DRAG     Pan freely (untracking any prev. focus obj.)\n"
		<< "SHIFT+MOUSE M. Pan with the focus pt./obj locked to the mouse\n"
		<< "F11            Toggle fullscreen\n"
		<< "F12            Toggle (most) HUD overlays\n"
		<< "-------------- God Mode (\"metaphysics\"):\n"
		<< "TAB            Toggle object interactions\n"
		<< "G              Gravity mode\n"
	//	<< "F              Decrease friction, +SHIFT: increase\n"
        //	<< "C              chg. collision mode: pass/stick/bounce\n"
		<< "-------------- Admin:\n"
		<< "F2-F8          Quicksave (overwrites!), +SHIFT: qickload\n"
		<< "M              Toggle (mute/unmute) audio\n"
		<< "SHIFT+M        Toggle music\n"
		<< "SHIFT+N        Toggle sound fx. \"noise\"\n"
//!! #543	<< "SHIFT+P        Performance (FPS) throttling on/off\n"
		<< "RIGHT ALT      Stream debug info to the terminal\n"
		<< "\n"
		<< "ESC            Quit\n"
		<< "\n"
		<< "-------------- Misc:\n"
		<< "?, F1          This help\n"
		<< "Command-line options: " << args.exename() << " /?"

	;
//cerr << help_hud;
}

#endif // DISABLE_HUDS


//----------------------------------------------------------------------------
void OONApp::onResize(unsigned width, unsigned height) //override
//!!Sink this into the UI!
{
//cerr << "onResize...\n"; //!!TBD: Not called on init; questionable
#ifndef DISABLE_HUDS
	ui_gebi(TimingStats).onResize(width, height);
	ui_gebi(WorldData)  .onResize(width, height);
	ui_gebi(ViewData)   .onResize(width, height);
	ui_gebi(ObjMonitor) .onResize(width, height);
	ui_gebi(HelpPanel)  .onResize(width, height);
	ui_gebi(Debug)      .onResize(width, height);
#endif
	gui.setPosition(4, main_window_height() - gui.getSize().y() - 4);
}

} // namespace OON
