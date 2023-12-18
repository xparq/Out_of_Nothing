#include "OON_sfml.hpp"

//!! This "backend tunelling" should be "allowed" (even properly facilitated,
//!! in a more way) later, after the backend selection becomes more transparent
//!! and/or automatic etc. (#294; finishing what I've started in SFW).
//!!
//!! A side-note/reminder: the goal here (on the app level) is only separating
//!! (most of?) the *sources* from direct backend dependencies ("write once"),
//!! not the entire compilation process.
//!!
//!! This is so sad, still...:
#include "Engine/Backend/_adapter_switcher.hpp"
#include SWITCHED(BACKEND, _Backend.hpp)
#define SFML_WINDOW() (((SFML_Backend&)backend).SFML_window())
#define SFML_HUD(x) (((UI::HUD_SFML&)backend).SFML_window())


import Storage;

#include "UI/adapter/SFML/keycodes.hpp" // SFML -> SimApp keycode translation

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Sleep.hpp>

#include <thread>
#include <mutex>
#include <memory>
	using std::make_shared;
#include <cstdlib>
	using std::rand; // + RAND_MAX (macro!)
#include <charconv>
	using std::to_chars;
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>

#include "extern/iprof/iprof.hpp"

using namespace Szim;

using namespace Model;
using namespace View;
using namespace UI;
using namespace sz;
using namespace std;


//============================================================================
namespace sync {
	std::mutex Updating; //!!?? Updating what, when, by whom?
};


//============================================================================
namespace OON {

//----------------------------------------------------------------------------
OON_sfml::OON_sfml(int argc, char** argv)
	: OONApp(argc, argv)
#ifndef DISABLE_HUD
//#define CFG_HUD_COLOR(cfgprop, def) (uint32_t(sfw::Color(appcfg.get(cfgprop, def)).toInteger()))
	// NOTE: .cfg is ready to use now!
	, timing_hud(SFML_WINDOW(),{ .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/timing_left", -250), .panel_top = appcfg.get("appearance/HUD/timing_top", 10),
		.fgcolor = appcfg.get("appearance/HUD/timing_fg", HUD::DEFAULT_TEXT_COLOR),
		.bgcolor = appcfg.get("appearance/HUD/timing_bg", HUD::DEFAULT_BACKGROUND_COLOR)})
	, world_hud(SFML_WINDOW(), { .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/world_state_left", -250), .panel_top = appcfg.get("appearance/HUD/world_state_top", 316),
		.fgcolor = appcfg.get("appearance/HUD/world_state_fg", 0x90e040ffu),
		.bgcolor = appcfg.get("appearance/HUD/world_state_bg", 0x90e040ffu/4)})
	, view_hud(SFML_WINDOW(), { .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/view_state_left", -250), .panel_top = appcfg.get("appearance/HUD/view_state_top", 424),
		.fgcolor = appcfg.get("appearance/HUD/view_state_fg", 0x90e040ffu),
		.bgcolor = appcfg.get("appearance/HUD/view_state_bg", 0x90e040ffu/4)})
	, object_hud(SFML_WINDOW(), { .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/object_monitor_left", -250), .panel_top = appcfg.get("appearance/HUD/object_monitor_top", 516),
		.fgcolor = appcfg.get("appearance/HUD/object_monitor_fg", 0xaaaaaaffu),
		.bgcolor = appcfg.get("appearance/HUD/object_monitor_bg", 0x33333340u)})
	, help_hud( SFML_WINDOW(), { .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/help_left", 10), .panel_top = appcfg.get("appearance/HUD/help_top", 10),
		.fgcolor = appcfg.get("appearance/HUD/help_fg", 0x40d040ffu),
		.bgcolor = appcfg.get("appearance/HUD/help_bg", 0x40f040ffu/4)})
	, debug_hud(SFML_WINDOW(), { .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/debug_left", -250), .panel_top = appcfg.get("appearance/HUD/debug_top", -350),
		.fgcolor = appcfg.get("appearance/HUD/debug_fg", 0x90e040ffu),
		.bgcolor = appcfg.get("appearance/HUD/debug_bg", 0x90e040ffu/4)})
#endif
{
}

//----------------------------------------------------------------------------
//!! Move this to SimApp, but only together with its counterpart in the update loop!
//!! Note that resetting the iter. counter and the model time should pro'ly be associated
//!! with run(), which should then be non-empty in SimApp, and should also somehow
//!! bring with it some main-loop logic to handle basic chores like time control & stepping!
//!! Perhaps most of that ugly & brittle `update_thread_main_loop()` could be moved there,
//!! and then updates_for_next_frame() could be an app callback (plus some new ones, handling
//!! that Window Context bullshit etc.), and its wrapping in SimApp could hopefully handle the timing stuff.
void OON_sfml::time_step(int steps)
{
	// Override the loop count limit, if reached (this may not always be applicable tho!); -> #216
	if (iterations.maxed())
		++iterations.limit;

	timestepping = steps; //! See resetting it in updates_for_next_frame()!
}

//----------------------------------------------------------------------------
void OON_sfml::update_thread_main_loop()
{
//	sf::Context context; //!! Seems redundant, as it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details
	                     //!! The only change I can see is a different getActiveContext ID here, if this is enabled.

#ifndef DISABLE_THREADS
	std::unique_lock proc_lock{sync::Updating, std::defer_lock};

	while (!terminated()) {
#endif
		switch (ui_event_state) {
		case UIEventState::BUSY:
//cerr << " [[[...BUSY...]]] ";
			break;
		case UIEventState::IDLE:
/*!! THIS HELPED NOTHING HERE:
			if (paused())
				sf::sleep(sf::milliseconds(100)); // Sleep even more, if *really* idle! :)
			// fallthrough
!!*/
		case UIEventState::EVENT_READY:
#ifndef DISABLE_THREADS
			try { proc_lock.lock(); } // Blocks
			catch (...) {
cerr << "- Oops! proc_lock.lock() failed! (already locked? " << proc_lock.owns_lock() << ")\n";
			}
#endif

			poll_controls(); // Should follow update_keys_from_SFML() (or else they'd get out of sync by some thread-switching delay!), until that's ensured implicitly!
			updates_for_next_frame();

			//!!?? Why is this redundant?!
			if (!SFML_WINDOW().setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [update_thread_main_loop] sf::setActive(true) failed!\n";
//?				request_exit(-1);
//?				return;
			}
			//!! This could indeed deadlock the entire process: while (!SFML_WINDOW().setActive(true));

			//!! This is problematic, as it currently relies on sf::setFrameRateLImit()
			//!! which would make the thread sleep -- but with still holding the lock! :-/
			//!! So... Either control the framerate ourselves (the upside of which is one
			//!! less external API dependency), or further separate rendering from actual
			//!! displaying (so we can release the update lock right after renedring)
			//!! -- AND THEN ALSO IMPLEMENTING SYNCING BETWEEN THOSE TWO!...
			draw();

			if (!SFML_WINDOW().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [update_thread_main_loop] sf::setActive(false) failed!\n";
//?				request_exit(-1);
//?				return;
			}
			//!! This could indeed deadlock the entire process: while (!SFML_WINDOW().setActive(false));

#ifndef DISABLE_THREADS
			try { proc_lock.unlock(); } // This can throw, too!
			catch (...) {
cerr << "- WTF: proc_lock.unlock() failed?! (already unlocked? " << !proc_lock.owns_lock() << ")\n";
			}
#endif
			break;
		default:
			assert(("[[[...!!UNKNOWN EVENT STATE!!...]]]", false));
		}

		// Drop the frame rate and/or sleep more if paused
		if (paused()) {
			sf::sleep(sf::milliseconds(
				cfg.get("sim/timing/paused_sleep_time_per_cycle", 40) // #330
			)); //!! or 100 for 10 FPS... But see #217! :-o
		}


//cerr << "- releasing Events...\n";
		//sync::EventsFreeToGo.release();

	IPROF_SYNC;
	IPROF_SYNC_THREAD;

/* Doing it with setFramerateLimit() now!
	//! If there's still time left from the frame slice:
	sf::sleep(sf::milliseconds(30)); //!! (remaining_time_ms)
		//! This won't stop the other (e.g. event loop) thread(s) from churning, though!
		//! -> use blocking/sleeping event query there!
		//! Nor does it ruin the smooth rendering! :-o WTF?!
		//! -> Because SFML double-buffers implicitly, AFAIK...
*/
//cerr << "sf::Context [update loop]: " << sf::Context::getActiveContextId() << endl;
#ifndef DISABLE_THREADS
	}
#endif
}

//----------------------------------------------------------------------------
void OON_sfml::draw() // override
{
	renderer.render(*this);
		// Was in updates_for_next_frame(), but pause should not stop UI updates.
		//!!...raising the question: at which point should UI rendering be separated from world rendering?

//#ifdef DEBUG
	if (!(keystate(ALT) && keystate(CTRL) /*&& keystate(Z)*/)) // -> #225
//#endif
		SFML_WINDOW().clear();

	renderer.draw(*this);

#ifndef DISABLE_HUD
	if (_show_huds) {
		timing_hud.draw(SFML_WINDOW());
		world_hud.draw(SFML_WINDOW());
		view_hud.draw(SFML_WINDOW());
		object_hud.draw(SFML_WINDOW());
		debug_hud.draw(SFML_WINDOW());
		if (help_hud.active())
			help_hud.draw(SFML_WINDOW()); //!! This active-chk is redundant: HUD::draw() does the same. TBD: who's boss?
		                                      //!! "Activity" means more than just drawing, so... (Or actually both should control it?)
	}
#endif

/*cerr << std::boolalpha
	<< "wallpap? "<<gui.hasWallpaper() << ", "
	<< "clea bg? "<<sfw::Theme::clearBackground << ", "
	<< hex << sfw::Theme::bgColor.toInteger() << '\n';
*/
        gui.render(); // Draw last, as a translucent overlay!

	SFML_WINDOW().display();
}

//----------------------------------------------------------------------------
void OON_sfml::updates_for_next_frame()
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
				if (e.lifetime != World::Body::Unlimited && e.lifetime <= 0) {
					remove_body(i); // Takes care of "known" references, too!
				}
			}

		} else {
			if (cfg.exit_on_finish) {
				cerr << "Exiting (as requested): iterations finished.\n";
				request_exit();
				// fallthrough
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
	// - Auto-scroll to follow pinned player/object
	// - Manual panning
	// - Zoom

	ui_gebi(ObjectData).active(focused_entity_ndx != ~0u && focused_entity_ndx < entity_count());
//if (!ui_gebi(ObjectData).active()) {
//cerr << "----- focused_entity_ndx: "<<focused_entity_ndx<<'\n';
//}


	auto _focus_locked_ = false;
	if (scroll_locked()) {
		// Panning follows focused obj. with locked focus point:
		_focus_locked_ = true;
		if (focused_entity_ndx != ~0u)
			follow_entity(focused_entity_ndx);
	} else {
		// Focus point follows focused obj., with panning only if drifting off-screen:
		if (focused_entity_ndx != ~0u) {
static const float autofollow_margin    = appcfg.get("controls/autofollow_margin", 100.f);
static const float autofollow_throwback = appcfg.get("controls/autofollow_throwback", 2.f);
static const float autozoom_delta       = appcfg.get("controls/autozoom_rate", 0.1f);
			oon_main_camera().focus_offset = oon_main_camera().world_to_view_coord(entity(focused_entity_ndx).p);
			if (oon_main_camera().confine(entity(focused_entity_ndx).p,
			    autofollow_margin + autofollow_margin/2 * oon_main_camera().scale()/Szim::SimAppConfig::DEFAULT_ZOOM,
			    autofollow_throwback)) { // true = drifted off
				zoom_control(AutoFollow, -autozoom_delta); // Emulate the mouse wheel...
//cerr << "oon_main_camera().scale(): "<<oon_main_camera().scale()<<", DEFAULT_ZOOM: "<<oon_main_camera().scale()<<", ratio: "<<oon_main_camera().scale() / Szim::SimAppConfig::DEFAULT_ZOOM<<'\n';
			}
		}
	}
	// Update the focus lock indicator:
	sfw::getWidget<sfw::CheckBox>("Pan follow object")->set(_focus_locked_);

	view_control(); // Manual view adjustments
}


//----------------------------------------------------------------------------
void OON_sfml::pause_hook(bool)
{
	//!! As a quick hack, time must restart from 0 when unpausing...
	//!! (The other restart() on pausing is redundant; just keeping it simple...)
cerr << "- INTERNAL: Main clock restarted on pause on/off (in the pause-hook)!\n";
	backend.clock.restart();
}


//----------------------------------------------------------------------------
void OON_sfml::event_loop()
{
	sf::Context context; //!! Seems redundant; it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details

	std::unique_lock noproc_lock{sync::Updating, std::defer_lock};

try {
	while (!terminated() && SFML_WINDOW().isOpen()) {

		for (sf::Event event; !terminated() && SFML_WINDOW().pollEvent(event);) {
		// This inner loop is here to prevent event "jamming" (delays in
		// event processing -- or even loss?) due to accumulating events
		// coming faster than 1/frame for a long enough period to cause
		// noticable jam/stutter.
		//
		// In non-threaded mode, very high event stream density can also
		// cause problems on its own, starving the rest of the main loop
		// from frequent-enough updates. So, there should be some sort of
		// triaging in those cases of overload, a balance between losing
		// some events and losing some updates.
		// (-- BUT THAT'S NOT IMPLEMENTED FOR NOW. JUST USE THREADING!
		// Overloads will happen there, too, obviously, but at least
		// the input and output processing will share the suffering. :) )
		//!!
		//!! BUT... I'm afraid, with the current crude thread-locking
		//!! model updates/reactions can still get locked out unfairly!
		//!!
			ui_event_state = UIEventState::BUSY;
#ifndef DISABLE_THREADS
//!! waitEvent was kinda elegant, but not very practical... Among other things,
//!! it can't be interrupted by our own internal "events", like request_exit()...
//!! Also, in both SFML and SDL, they already have to do pollEvents internally anyway:
//!! -> https://en.sfml-dev.org/forums/index.php?topic=18264.0
//!!		if (!SFML_WINDOW().waitEvent(event)) {
//!!			cerr << "- Event processing failed. WTF?! Terminating.\n";
//!!			exit(-1);
//!!		}

//cerr << "- acquiring lock for events...\n";
			noproc_lock.lock();
#endif
			if (!SFML_WINDOW().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [event_loop] sf::setActive(false) failed!\n";
//?				request_exit(-1);
//?				return;
			}

			//!! The update thread may still be busy calculating, so we can't just go ahead and change things!
			//!! But... then again, how come this thing still works at all?! :-o
			//!! Clearly there must be cases when processing the new event here overlaps with an ongoing update
			//!! cycle before they notice our "BUSY" state change here, and stop! :-o
			//!! So... a semaphore from their end must be provided to this point of this thread, too!
			//!! And it may not be just as easy as sg. like:
			//!!while (game.busy_updating())
			//!!	;
			//!! A much cleaner way would be pushing the events received here
			//!! into a queue in the update/processing thread, like:
			//!! game.inputs.push(event);
			//!! (And then the push here and the pop there must be synchronized -- hopefully just <atomic> would do.)

			UI::update_keys_from_SFML(event); // Using the SFML adapter (via #include UI/adapter/SFML/...)
				//!! This should be generalized beyond keys, and should also make it possible
				//!! to use abstracted event types/codes for dispatching (below)!

//!! It felt more uneven if done here (due to the too coarse thread granularity of Windows -- and/or my own botched threading logic)! :-o
//!!			poll_controls(); // Should follow update_keys_from_SFML() (or else they'd get out of sync by some thread-switching delay!), until that's ensured implicitly!

			// Close req.?
			if (event.type == sf::Event::Closed ||
			    event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				request_exit();
				// [fix-setactive-fail] -> DON'T: window.close();
				//!!?? I forgot: how exactly is the window being closed on Esc?
				//!!?? I *guess* by the sf::Window dtor, but why do I vaguely
				//!!?? recall endless annoying problems with that from earlier?!
				break;
			}

			// If the GUI has the input focus, let it process the event
			// -- except for some that really don't belong there:
			if (gui.focused() &&
				event.type != sf::Event::LostFocus && // Yeah, so this is an entirely different "focus"! :-o
				event.type != sf::Event::GainedFocus &&
				(event.type != sf::Event::MouseButtonPressed ||
				 event.type == sf::Event::MouseButtonPressed && gui.contains(gui.getMousePosition()))) //!!{event.mouseButton.x, event.mouseButton.y})))
			{
				goto process_ui_event;
			}
			// Else:
			gui.unfocus(); // A bit hamfisted, but: the event is ours, let the UI know!...
			switch (event.type) //!! See above: morph into using abstracted events!
			{
			case sf::Event::KeyPressed:
#ifdef DEBUG
	if (cfg.DEBUG_show_keycode) cerr << "key code: " << event.key.code << "\n";
#endif
				switch (event.key.code) {
				case sf::Keyboard::Pause: toggle_pause(); break;
				case sf::Keyboard::Enter: time_step(1); break;
				case sf::Keyboard::Backspace: time_step(-1); break;

				case sf::Keyboard::Tab: toggle_interact_all(); break;

				case sf::Keyboard::Insert: spawn(player_entity_ndx(),
						keystate(SHIFT) ? 100 : keystate(CTRL) ? 10 : 1); break;
				case sf::Keyboard::Delete: remove_random_bodies(
						keystate(SHIFT) ? 100 : keystate(CTRL) ? 10 : 1); break;

				case sf::Keyboard::F1:  keystate(SHIFT) ? quick_load_snapshot(1) : quick_save_snapshot(1); break;
				case sf::Keyboard::F2:  keystate(SHIFT) ? quick_load_snapshot(2) : quick_save_snapshot(2); break;
				case sf::Keyboard::F3:  keystate(SHIFT) ? quick_load_snapshot(3) : quick_save_snapshot(3); break;
				case sf::Keyboard::F4:  keystate(SHIFT) ? quick_load_snapshot(4) : quick_save_snapshot(4); break;
				case sf::Keyboard::F5:  keystate(SHIFT) ? quick_load_snapshot(5) : quick_save_snapshot(5); break;
				case sf::Keyboard::F6:  keystate(SHIFT) ? quick_load_snapshot(6) : quick_save_snapshot(6); break;
				case sf::Keyboard::F7:  keystate(SHIFT) ? quick_load_snapshot(7) : quick_save_snapshot(7); break;
				case sf::Keyboard::F8:  keystate(SHIFT) ? quick_load_snapshot(8) : quick_save_snapshot(8); break;

				case sf::Keyboard::Home:
					if (keystate(CTRL)) {
						//!! These should be "upgraded" to "Camera/view reset"!
						//!! oon_main_camera().reset() already exists:
						oon_main_camera().reset(); //!! ...but is not enough. :-/
						pan_reset();         //!! And these also overlap the camera reset...
						zoom_reset();        //!! SO, RECONCILE THEM!
					} else if (keystate(SHIFT)) {
						if (focused_entity_ndx != ~0u)
							center_entity(focused_entity_ndx);
					} else {
						center_player();
						focused_entity_ndx = player_entity_ndx();
					}
					break;

				case sf::Keyboard::F12: toggle_huds();
					sfw::getWidget<sfw::CheckBox>("Show HUDs")->set(huds_active());
					break;
				case sf::Keyboard::F11:
					toggle_fullscreen();
					//!! Refresh all our own (app-level) dimensions, too!
					//!! E.g. #288, and wrong .view size etc.!...
					break;

//				default:
//cerr << "UNHANDLED KEYPRESS: " << event.key.code << endl;
				}
				break;

			case sf::Event::TextEntered:
				if (event.text.unicode > 128) break; // non-ASCII!
				switch (static_cast<char>(event.text.unicode)) {
				case 'g':
					sfw::getWidget<sfw::OptionsBox<Model::World::GravityMode>>("Gravity mode")->selectNext();
					break;
//				case 'f': world().friction -= 0.01f; break;
//				case 'F': world().friction += 0.01f; break;
				case 'r': time.reversed = !time.reversed; break;
				case 't': time.scale *= 2.0f; break;
				case 'T': time.scale /= 2.0f; break;
				case 'h': toggle_pause(); break;
				case 'M': toggle_muting();
					sfw::getWidget<sfw::CheckBox>("Audio: ")->set(backend.audio.enabled);
					break;
				case 'm': toggle_music(); break;
				case 'n': toggle_sound_fx();
					sfw::getWidget<sfw::CheckBox>(" - FX: ")->set(backend.audio.fx_enabled);
					break;
				case 'P': fps_throttling(!fps_throttling()); break;
				case 'x': toggle_fixed_model_dt();
					sfw::getWidget<sfw::CheckBox>("Fixed model Δt")->set(cfg.fixed_model_dt_enabled);
					break;
				case '?': toggle_help(); break;
				}
				break;
/*!!NOT YET, AND NOT FOR SPAWN (#83):
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					spawn(globe_ndx, 100);
				}
				break;
!!*/
			case sf::Event::MouseWheelScrolled:
			{
				//!! As a quick workaround for #334, we just check the GUI rect here
				//!! directly and pass the event if it belongs there...
//sf::Vector2f mouse = gui.getMousePosition() + gui.getPosition();
//cerr << "-- mouse: " << mouse.x <<", "<< mouse.y << "\n";
				if (gui.focused() || gui.contains(gui.getMousePosition()))
					goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				view_control(event.mouseWheelScroll.delta); //! Apparently always 1 or -1...
//renderer.p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4;
				break;
			}

			case sf::Event::MouseButtonPressed:
			{
//sf::Vector2f mouse = gui.getMousePosition() + gui.getPosition();
//cerr << "-- mouse: " << event.mouseButton.x <<", "<< event.mouseButton.y << "\n";

//!!??auto vpos = oon_main_camera().screen_to_view_coord(x, y); //!!?? How the FUCK did this compile?!?!? :-o
//!!?? Where did this x,y=={-520,-391} come from?! :-ooo
cerr << "???? x = " << x << ", y = " << y << " <-- WHAT THE HELL ARE THESE??? :-ooo\n";

				//!! As a quick workaround for #334, we just check the GUI rect here
				//!! directly and pass the event if it belongs there...
				if (gui.contains(gui.getMousePosition()))
					goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				Math::Vector2f vpos = oon_main_camera().screen_to_view_coord(event.mouseButton.x, event.mouseButton.y);
				oon_main_camera().focus_offset = vpos;
				size_t clicked_entity_id = ~0u;
				if (entity_at_viewpos(vpos.x, vpos.y, &clicked_entity_id)) {
cerr << "- Following object #"<<clicked_entity_id<<" now...\n";
				} else {
cerr << "- Nothing there, focusing on the deep void...\n";
				}
				focused_entity_ndx = scroll_locked()
					? (clicked_entity_id == ~0u ? player_entity_ndx() : clicked_entity_id)
					: clicked_entity_id; // ~0u if none... //!!... Whoa! :-o See updates_for_next_frame()!
				break;
			}

			case sf::Event::LostFocus:
				reset_keys();
				renderer.p_alpha = Renderer_SFML::ALPHA_INACTIVE;
				break;

			case sf::Event::GainedFocus:
				reset_keys();
				renderer.p_alpha = Renderer_SFML::ALPHA_ACTIVE;
				break;

			default:
process_ui_event:		// The GUI should be given a chance before this `switch`, but... -> #334: it can't swallow events!
				gui.process(event);
				//!! Also, it's kinda inconsistent with this `IDLE` state assumption below!...
				//!! (Hopefully it's not even used nowadays at all though...)
				ui_event_state = UIEventState::IDLE;

				break;
			} // switch

//cerr << "sf::Context [event loop]: " << sf::Context::getActiveContextId() << endl;

			ui_event_state = UIEventState::EVENT_READY;

#ifdef DISABLE_THREADS
		} // for - events in the queue

		update_thread_main_loop(); // <- Doesn't actually loop, when threads are disabled, so crank it from here!

//!!test idempotency:
//!!	draw();

#else
//cerr << "- freeing the proc. lock...\n";
			noproc_lock.unlock();

			//! Sleep some here, too (not just outside this inner loop, while
			//! idling), counterintuitively *especially* while jamming, in order to
			//! give the actual processing parts some unlocked time!...
			sf::sleep(sf::milliseconds(5)); //!! Should be adaptive!
		} // for - events in the queue

		// The event queue has been emptied, so in this thread (of input processing)
		// we're idling now (while updates are happening elsewhere), so:
		//!! Should be adaptive!!
		sf::sleep(sf::milliseconds(
			paused() ? cfg.get("sim/timing/paused_sleep_time_per_cycle", 50) // #330
			         : 10)); // SFML does (used to?) sleep the same amount for waitEvent
			// NOTE: This is only relevant when threading!
			// The non-threaded main loop sleeps via backend.hci.fps_throttling.
			//!!?? How about fps throttling *with* threading then?! -> #217

		if (paused() && ui_event_state == UIEventState::IDLE)
		{
			sf::sleep(sf::milliseconds(100)); // Sleep even more, if *really* idle! :)
		}

#endif			
	} // while - still running

} catch (runtime_error& x) {
	cerr << __FUNCTION__ " - ERROR: " << x.what() << '\n';
	return;
} catch (exception& x) {
	cerr << __FUNCTION__ " - EXCEPTION: " << x.what() << '\n';
	return;
} catch (...) {
	cerr << __FUNCTION__ " - UNKNOWN EXCEPTION!\n";
	return;
}
}


//----------------------------------------------------------------------------
size_t OON_sfml::add_body(World::Body&& obj) //override
{
	auto ndx = OONApp::add_body(std::forward<decltype(obj)>(obj));
	// Pre-cache shapes for rendering... (!! Likely pointless, but this is just what I started with...)
	renderer.create_cached_body_shape(*this, obj, ndx);
	return ndx;
}

//----------------------------------------------------------------------------
void OON_sfml::remove_body(size_t ndx) //override
{
	OONApp::remove_body(ndx);
	renderer.delete_cached_body_shape(*this, ndx);
}

void OON_sfml::resize_shapes(float factor) //override
{
	renderer.resize_objects(factor);
}

void OON_sfml::resize_shape(size_t ndx, float factor) //override
{
	renderer.resize_object(ndx, factor);
}


//----------------------------------------------------------------------------
bool OON_sfml::load_snapshot(const char* fname) //override
{
	// This should load the model back, but can't rebuild the rendering state:
	if (!SimApp::load_snapshot(fname)) {
		return false;
	}

	//!! NOPE: set_world(world_snapshots[slot]);
	//! Alas, somebody must resync the renderer, too!... :-/
/* A cleaner, but slower way would be:
	//! 1. Halt everything...
	//     DONE, for now, by the event handler!
	//! 2. Purge everything...
	remove_bodies();
	//! 3. Add the bodies back...
	for (auto& bodyptr : world_snapshots[slot]) {
		add_body(*bodyptr);
	}
*/// Faster, more under-the-hood method:
	//! 1. Halt everything...
	//     DONE, for now, by the event handler!
	//! 2. Purge the renderer only...
	renderer.reset();
	//! 3. Recreate the shapes...
	for (size_t n = 0; n < const_world().bodies.size(); ++n) {
		renderer.create_cached_body_shape(*this, *world().bodies[n], n);
	}
// The engine has already written that:
//cerr << "Game state restored from \"" << fname << "\".\n";
	return true;
}

} // namespace OON
