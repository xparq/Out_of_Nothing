﻿#include "OON_sfml.hpp"

//!! This could be "allowed" later directly in OON.cpp, too, after the backend selection
//!! becomes more transparent/automatic (e.g. finishing what I've started in SFW),
//!! because the goal here (on the app level) is only separating the source from the
//!! backend dependencies ("write once"), not the compilation process!
#include "Engine/Backend/SFML/_Backend.hpp"
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
	std::mutex Updating;
};


//============================================================================
//----------------------------------------------------------------------------
OON_sfml::OON_sfml(int argc, char** argv) : OON(argc, argv)
#ifndef DISABLE_HUD
	// NOTE: .cfg is ready to use now!
	, timing_hud(SFML_WINDOW(), cfg.asset_dir + cfg.hud_font_file, -250, 10)
	, debug_hud(SFML_WINDOW(), cfg.asset_dir + cfg.hud_font_file, -250, 260, 0x90e040ff, 0x90e040ff/4)
	, help_hud( SFML_WINDOW(), cfg.asset_dir + cfg.hud_font_file, 10, 10, 0x40d040ff, 0x40f040ff/4) // left = 10
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
	assert(timestepping == steps); // sz::Counter op= was very fiddly, I still don't quite trust it 100%!...
}

//----------------------------------------------------------------------------
void OON_sfml::update_thread_main_loop()
{
//	sf::Context context; //!! Seems redundant, as it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details
	                     //!! The only change I can see is a different getActiveContext ID here, if this is enabled.

	std::unique_lock proc_lock{sync::Updating, std::defer_lock};

#ifndef DISABLE_THREADS
	while (!terminated()) {
#endif
		switch (ui_event_state) {
		case UIEventState::BUSY:
//cerr << " [[[...BUSY...]]] ";
			break;
		case UIEventState::IDLE:
		case UIEventState::EVENT_READY:
			try {
				proc_lock.lock();
			} catch (...) {
				cerr << "- WTF proc_lock failed?! (already locked? " << proc_lock.owns_lock() << ")\n";
			}
			updates_for_next_frame();
			//!!?? Why is this redundant?!
			if (!SFML_WINDOW().setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [update_thread_main_loop] sf::setActive(true) failed!\n";
//?				terminate();
//?				return;
			}

			//!! This is problematic, as it currently relies on sf::setFrameRateLImit()
			//!! which would make the thread sleep -- but with still holding the lock! :-/
			//!! So... Either control the framerate ourselves (the upside of which is one
			//!! less external API dependency), or further separate rendering from actual
			//!! displaying (so we can release the update lock right after renedring)
			//!! -- AND THEN ALSO IMPLEMENTING SYNCING BETWEEN THOSE TWO!...
			draw();
			if (!SFML_WINDOW().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [update_thread_main_loop] sf::setActive(false) failed!\n";
//?				terminate();
//?				return;
			}
			proc_lock.unlock();
			break;
		default:
			assert(("[[[...!!UNKNOWN EVENT STATE!!...]]]", false));
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
		debug_hud.draw(SFML_WINDOW());
		if (help_hud.active()) help_hud.draw(SFML_WINDOW()); //!! the active-chk is redundant, the HUD does the same; TBD, who's boss!
		                                              //!! "activity" may mean more than drawing. so... actually both can do it?
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

	if (paused()) {
		if (!timestepping) { // Are we single-stepping?
			sf::sleep(sf::milliseconds(50)); //!! That 50 should drop the frame rate to ~15-20 FPS... But see #217! :-o
			return;
		}
	}

	//!!? Get some fresh immediate (continuous) input control state updates,
	//!!? in addition to the async. event_loop()!...
	poll_and_process_controls();

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
	//!! Move to a SimApp virtual, I guess (so at leat the counter capping can be implicitly done there; see also time_step()!):
	if (!iterations.maxed()) {
		update_world(Δt);
		++iterations;
	}

	// One less step to make next time (if any):
	if (timestepping) if (timestepping < 0 ) ++timestepping; else --timestepping;

	// Auto-scroll to follow player movement...
	//!
	//! NOTE: THIS MUST COME AFTER RECALCULATING THE NEW STATE!
	//!
	auto _locked_tmp = false;
	if (keystate(SCROLL_LOCKED) || keystate(SHIFT) || ((sfw::CheckBox*)gui.recall("Pan override"))->get()) {
		pan_to_player();
		_locked_tmp = true;
	}
	// Update the pan lock indicator:
	((sfw::CheckBox*)gui.recall(" - pan locked"))->set(_locked_tmp);
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
	while (SFML_WINDOW().isOpen() && !terminated()) {
			sf::Event event;

#ifndef DISABLE_THREADS
		if (!SFML_WINDOW().waitEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}

		ui_event_state = UIEventState::BUSY;
//cerr << "- acquiring lock for events...\n";
		noproc_lock.lock();

#else
/*!!?? Why did this (always?) fail?!
		if (!SFML_WINDOW().pollEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}
??!!*/			

		// This inner loop is only for non-threading mode, to prevent event processing
		// (reaction) delays (or even loss) due to accumulating events coming faster
		// than 1/frame (for a long enough period to cause noticable jam/stutter)!
		for (; SFML_WINDOW().pollEvent(event);) {

		ui_event_state = UIEventState::BUSY;
//cerr << "- acquiring events...\n";
#endif
			if (!SFML_WINDOW().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [event_loop] sf::setActive(false) failed!\n";
//?				terminate();
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

			UI::update_keys(event); // Using the SFML adapter (via #include UI/adapter/SFML/...)
				//!! This should be generalized beyond keys, and should also make it possible
				//!! to use abstracted event types/codes for dispatching (below)!

			switch (event.type) //!! See above: morph into using abstracted events!
			{
			case sf::Event::KeyPressed:
//!!See main.cpp:
#ifdef DEBUG
	if (cfg.DEBUG_show_keycode) cerr << "key code: " << event.key.code << "\n";
#endif

				switch (event.key.code) {
				case sf::Keyboard::Escape: //!!Merge with Closed!
					terminate();
					// [fix-setactive-fail] -> DON'T: window.close();
					break;

				case sf::Keyboard::Pause: toggle_pause(); break;
				case sf::Keyboard::Enter: time_step(1); break;
				case sf::Keyboard::Backspace: time_step(-1); break;

				case sf::Keyboard::Tab: toggle_interact_all(); break;

				case sf::Keyboard::Insert: spawn(player_entity_ndx(), keystate(SHIFT) ? 1 : 100); break;
//!!...			case sf::Keyboard::Insert: add_bodies(keystate(SHIFT) ? 1 : 100); break;
				case sf::Keyboard::Delete: remove_bodies(keystate(SHIFT) ? 1 : 100); break;
//!!??			case sf::Keyboard::Delete: OON::remove_body(); break; //!!??WTF is this one ambiguous (without the qualif.)?!

				case sf::Keyboard::F1:  keystate(SHIFT) ? load_snapshot(1) : save_snapshot(1); break;
				case sf::Keyboard::F2:  keystate(SHIFT) ? load_snapshot(2) : save_snapshot(2); break;
				case sf::Keyboard::F3:  keystate(SHIFT) ? load_snapshot(3) : save_snapshot(3); break;
				case sf::Keyboard::F4:  keystate(SHIFT) ? load_snapshot(4) : save_snapshot(4); break;

				case sf::Keyboard::Home:
					if (keystate(CTRL))
						pan_reset(); //!!Should be "upgraded" to "Camera/view reset" -- also resetting the zoom?
					else
						pan_to_player();
					break;

				case sf::Keyboard::F12: toggle_huds();
					((sfw::CheckBox*)gui.recall("Show HUDs"))->set(huds_active());
					break;
				case sf::Keyboard::F11:
					while (!SFML_WINDOW().setActive(true));
						//!!Investigating the switching problem (#190)...
						//!! - this "being careful" makes no diff.:
					toggle_fullscreen();
					SFML_WINDOW().setActive(false); // Don't loop this one, we'd get stuck here!
					break;

//				default:
//cerr << "UNHANDLED KEYPRESS: " << event.key.code << endl;
				}
				break;

			case sf::Event::TextEntered:
				if (event.text.unicode > 128) break; // non-ASCII!
				switch (static_cast<char>(event.text.unicode)) {
				case 'f': world().FRICTION -= 0.01f; break;
				case 'F': world().FRICTION += 0.01f; break;
				case '+': zoom_in(); break;
				case '-': zoom_out(); break;
				case 'r': time.reversed = !time.reversed; break;
				case 't': time.scale *= 2.0f; break;
				case 'T': time.scale /= 2.0f; break;
				case 'h': toggle_pause(); break;
				case 'M': toggle_muting();
					((sfw::CheckBox*)gui.recall("Audio: "))->set(backend.audio.enabled);
					break;
				case 'm': toggle_music(); break;
				case 'n': toggle_sound_fx();
					((sfw::CheckBox*)gui.recall(" - FX: "))->set(backend.audio.fx_enabled);
					break;
				case 'P': fps_throttling(!fps_throttling()); break;
				case 'x': toggle_fixed_model_dt();
					((sfw::CheckBox*)gui.recall("Fixed model Δt"))->set(cfg.fixed_model_dt_enabled);
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
				if (event.mouseWheelScroll.delta > 0) zoom_in(); else zoom_out();
//				renderer.p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4; //! seems to always be 1 or -1...
				break;

			case sf::Event::LostFocus:
				renderer.p_alpha = Renderer_SFML::ALPHA_INACTIVE;
				break;

			case sf::Event::GainedFocus:
				renderer.p_alpha = Renderer_SFML::ALPHA_ACTIVE;
				break;

			case sf::Event::Closed: //!!Merge with key:Esc!
				terminate();
//cerr << "BEGIN sf::Event::Closed\n"; //!!this frame is to trace an error from SFML/OpenGL
				SFML_WINDOW().close();
//cerr << "END sf::Event::Closed\n";
				break;

			default:
				gui.process(event);

				ui_event_state = UIEventState::IDLE;

				break;
			}

//cerr << "sf::Context [event loop]: " << sf::Context::getActiveContextId() << endl;

#ifndef DISABLE_THREADS
			ui_event_state = UIEventState::EVENT_READY;
//cerr << "- freeing proc lock...\n";
			noproc_lock.unlock();
#else
		} // for
		ui_event_state = UIEventState::EVENT_READY;
		update_thread_main_loop(); // <- not looping when threads disabled
//!!test idempotency:
//!!	draw();

#endif			
	} // while

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
size_t OON_sfml::add_body(World::Body&& obj)
{
	auto ndx = OON::add_body(std::forward<decltype(obj)>(obj));
	// Pre-cache shapes for rendering... (!! Likely pointless, but this is just what I started with...)
	renderer.create_cached_body_shape(*this, obj, ndx);
	return ndx;
}

//----------------------------------------------------------------------------
void OON_sfml::remove_body(size_t ndx)
{
	OON::remove_body(ndx);
	renderer.delete_cached_body_shape(*this, ndx);
}

void OON_sfml::post_zoom_hook(float factor)
{
	renderer.resize_objects(factor);
	_adjust_pan_after_zoom(factor);
}

//----------------------------------------------------------------------------
void OON_sfml::_adjust_pan_after_zoom(float factor)
{
	auto vpos = view.world_to_view_coord(player_model()->p) - view.offset;
	pan(-(vpos - vpos/factor));
}

/*
	// If the new zoom level would put the player object out of view, reposition the view so that
	// it would keep being visible; also roughly at the same view-offset as before!

	auto visible_R = player_model()->r * view.zoom; //!! Not a terribly robust method to get that size...

	if (abs(vpos.x) > cfg.VIEWPORT_WIDTH/2  - visible_R ||
	    abs(vpos.y) > cfg.VIEWPORT_HEIGHT/2 - visible_R)
	{
cerr << "R-viewsize: " << view.zoom * plm->r
	 << " abs(vpos.x): " << abs(vpos.x) << ", "
     << " abs(vpos.u): " << abs(vpos.y) << endl;

		pan_to_player(offset);
		pan_to_entity(player_entity_ndx(), vpos * CFG_ZOOM_CHANGE_RATIO); // keep the on-screen pos!
//		zoom_out(); //!! Shouldn't be an infinite zoom loop (even if moving way too fast, I think)
	}
*/


//----------------------------------------------------------------------------
bool OON_sfml::init() // override
{
	if (!OON::init()) {
		return false; // It should've dealt with any dignotstics already
	}

	return true;
}

//----------------------------------------------------------------------------
//!!Sink this into the UI!
void OON_sfml::onResize() // override
{
#ifndef DISABLE_HUD
	((UI::HUD_SFML&)debug_hud).onResize(((SFML_Backend&)backend).SFML_window());
	((UI::HUD_SFML&)help_hud) .onResize(((SFML_Backend&)backend).SFML_window());
#endif
}


bool OON_sfml::load_snapshot(unsigned slot_id) // starting from 1, not 0!
{
	// This should load the model back, but can't rebuild the rendering state:
	if (!SimApp::load_snapshot(slot_id)) {
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
cerr << "Game state restored from slot " << slot_id << ".\n";
	return true;
}