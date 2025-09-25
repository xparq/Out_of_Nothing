﻿// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "OON_sfml.hpp"
#include "OON_UI.hpp"

//!! This "backend tunneling" should be "allowed" (even properly facilitated,
//!! in a more civilized way) later, after the backend selection becomes more
//!! transparent and/or automatic etc. (#294; finishing what I've started in
//!! MycoGUI).
//!!
//!! A side-note/reminder: the goal here (on the app level) is only separating
//!! (most of?) the *sources* from direct backend dependencies ("write once"),
//!! not the entire compilation process.
//!!
//!! This is so sad, still...:
#include "Engine/Backend/_adapter_switcher.hpp"
#include SWITCHED(BACKEND, _Backend.hpp)
#define SFML_WINDOW() (((SFML_Backend&)backend).SFML_window())
#define SFML_KEY(KeyName) unsigned(sf::Keyboard::Key::KeyName) //!!XLAT


//!!GCC still doesn't like modules:
//!!import Storage; //!! Just a dummy (reminder, smoke test etc.) for now!

#include "Engine/UI/adapter/SFML/keycodes.hpp" // SFML -> SimApp keycode translation

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
#include <cassert>

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"

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

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
namespace _internal {
	FUCpp_ViewHack::FUCpp_ViewHack(OONApp_sfml& app) : _oon_view(app) {}
	FUCpp_ViewHack::_oon_view_container::_oon_view_container(OONApp_sfml& app)
	//!!WAS:	: oon_main_camera({.width  = (float)backend.hci.window().width,  //!!WAS: Szim::SimAppConfig::VIEWPORT_WIDTH, //!! Would (should!) be reset later from "real data" from the backend anyway...
	//!!WAS:	                   .height = (float)backend.hci.window().height, //!!WAS: Szim::SimAppConfig::VIEWPORT_HEIGHT,
	//!!WAS:	, _oon_main_view({.width = Szim::SimAppConfig::VIEWPORT_WIDTH,
	//!!WAS:	                  .height = Szim::SimAppConfig::VIEWPORT_HEIGHT},
		: _oon_main_view(app)
	{
	//LOGD << "_oon_view_and_cam_container: _oon_main_view.camera ptr: "<<&_oon_main_view.camera();
	}
}
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//----------------------------------------------------------------------------
OONApp_sfml::OONApp_sfml(const RuntimeContext& runtime, int argc, char** argv)
	: FUCpp_ViewHack(*this) // No Engine here to use for init. the View yet! :-/
	, OONApp(runtime, argc, argv, _oon_view._oon_main_view) //!! Ugh...
#ifndef DISABLE_HUDS
//#define CFG_HUD_COLOR(cfgprop, def) (uint32_t(myco::Color(appcfg.get(cfgprop, def)).toInteger()))
	// NOTE: .cfg is ready to use now!
	, timing_hud({ .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/timing_left", -250), .panel_top = appcfg.get("appearance/HUD/timing_top", 10),
		.fgcolor = appcfg.get("appearance/HUD/timing_fg", HUDStream::DEFAULT_TEXT_COLOR),
		.bgcolor = appcfg.get("appearance/HUD/timing_bg", HUDStream::DEFAULT_BACKGROUND_COLOR)})
	, world_hud({ .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/world_state_left", -250), .panel_top = appcfg.get("appearance/HUD/world_state_top", 314),
		.fgcolor = appcfg.get("appearance/HUD/world_state_fg", 0x90e040ffu),
		.bgcolor = appcfg.get("appearance/HUD/world_state_bg", 0x90e040ffu/4)})
	, view_hud({ .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/view_state_left", -250), .panel_top = appcfg.get("appearance/HUD/view_state_top", 420),
		.fgcolor = appcfg.get("appearance/HUD/view_state_fg", 0x90e040ffu),
		.bgcolor = appcfg.get("appearance/HUD/view_state_bg", 0x90e040ffu/4)})
	, object_hud({ .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/object_monitor_left", -250), .panel_top = appcfg.get("appearance/HUD/object_monitor_top", 526),
		.fgcolor = appcfg.get("appearance/HUD/object_monitor_fg", 0xaaaaaaffu),
		.bgcolor = appcfg.get("appearance/HUD/object_monitor_bg", 0x33333340u)})
	, help_hud( { .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/help_left", 10), .panel_top = appcfg.get("appearance/HUD/help_top", 10),
		.fgcolor = appcfg.get("appearance/HUD/help_fg", 0x40d040ffu),
		.bgcolor = appcfg.get("appearance/HUD/help_bg", 0x40f040ffu/4)})
	, debug_hud({ .font_file = cfg.asset_dir + appcfg.hud_font_file,
		.line_height  = appcfg.hud_line_height, .line_spacing = appcfg.hud_line_spacing,
		.panel_left = appcfg.get("appearance/HUD/debug_left", -250), .panel_top = appcfg.get("appearance/HUD/debug_top", -350),
		.fgcolor = appcfg.get("appearance/HUD/debug_fg", 0x90e040ffu),
		.bgcolor = appcfg.get("appearance/HUD/debug_bg", 0x90e040ffu/4)})
#endif
{
//std::cerr << "--- OONApp_sfml ctor" << std::endl;
}

//----------------------------------------------------------------------------
void OONApp_sfml::update_thread_main_loop()
{
//	sf::Context context; //!! Seems redundant, as it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details
	                     //!! The only change I can see is a different getActiveContext ID here, if this is enabled.

#ifndef DISABLE_THREADS
	std::unique_lock proc_lock{sync::Updating, std::defer_lock};

	while (!terminated()) {
#endif
		switch (ui_event_state) {
		case UIEventState::BUSY:
//LOGI << " [[[...BUSY...]]]";
			break;
		case UIEventState::IDLE:
/*!! THIS HELPED NOTHING HERE:
			if (paused())
				sf::sleep(sf::milliseconds(100)); // Sleep even more, if *really* idle! :)
			[[fallthrough]]
!!*/
		case UIEventState::EVENT_READY:
#ifndef DISABLE_THREADS
			try { proc_lock.lock(); } // Blocks
			catch (...) {
LOGE << "- Oops! proc_lock.lock() failed! (already locked? " << proc_lock.owns_lock() << ")";
			}
#endif

			get_control_inputs(); // Should follow update_keys_from_SFML() (or else they'd get out of sync by some thread-switching delay!), until that's ensured implicitly!
			updates_for_next_frame();

			if (!cfg.headless) {
				//!!?? Why is this redundant?!
				if (!SFML_WINDOW().setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
					LOGE << "\n- [update_thread_main_loop] sf::setActive(true) failed!";
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
				//!! Occasionally test idempotency by drawing again:
				//!!draw();

				if (!SFML_WINDOW().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
					LOGE << "\n- [update_thread_main_loop] sf::setActive(false) failed!";
	//?				request_exit(-1);
	//?				return;
				}
				//!! This could indeed deadlock the entire process: while (!SFML_WINDOW().setActive(false));
			}

#ifndef DISABLE_THREADS
			try { proc_lock.unlock(); } // This can throw, too!
			catch (...) {
LOGE << "- WTF: proc_lock.unlock() failed?! (already unlocked? " << !proc_lock.owns_lock() << ")";
			}
#endif
			break;
		default:
			assert(("[[[...!!UNKNOWN EVENT STATE!!...]]]" ?0:0));
		}

		// Drop the frame rate and/or sleep more if paused
		if (paused()) {
			sf::sleep(sf::milliseconds(
				cfg.get("sim/timing/paused_sleep_time_per_cycle", 40) // #330
			)); //!! or 100 for 10 FPS... But see #217! :-o
		}


//LOGD << "- releasing Events...";
		//sync::EventsFreeToGo.release();

//!!IPROF_SYNC_THREAD;

/* Doing it with setFramerateLimit() now!
	//! If there's still time left from the frame slice:
	sf::sleep(sf::milliseconds(30)); //!! (remaining_time_ms)
		//! This won't stop the other (e.g. event loop) thread(s) from churning, though!
		//! -> use blocking/sleeping event query there!
		//! Nor does it ruin the smooth rendering! :-o WTF?!
		//! -> Because SFML double-buffers implicitly, AFAIK...
*/
//LOGD << "sf::Context [update loop]: " << sf::Context::getActiveContextId();
#ifndef DISABLE_THREADS
	}
#endif
}

//----------------------------------------------------------------------------
void OONApp_sfml::draw() // override
//!!?? Is there a nice, exact criteria by which UI rendering can be distinguished from model rendering?
{
//#ifdef DEBUG
	if (!controls.ShowOrbits) // -> #225
//#endif
		SFML_WINDOW().clear();

	oon_main_view().draw(); //!! Change it to draw(surface)!

/*LOGD	<< std::boolalpha
	<< "wallpapr? "<<gui.hasWallpaper() << ", "
	<< "clear bg? "<<myco::Theme::clearBackground << ", "
	<< hex << myco::Theme::bgColor.toInteger();
*/
        gui.render(); // Draw last, as a translucent overlay!
//!! These are (will be...) also part of the GUI:
#ifndef DISABLE_HUDS
	if (_ui_show_huds) {
		auto& target = SFML_WINDOW(); //!! gui.window();
		myco::gfx::RenderContext ctx{ target };
		timing_hud.draw(ctx);
		world_hud.draw(ctx);
		view_hud.draw(ctx);
		object_hud.draw(ctx);
		debug_hud.draw(ctx);
		if (help_hud.active())
			help_hud.draw(ctx); //!!?? This active()-chk is redundant: HUD::draw() does the same. TBD: who's boss?
		                            //!!?? "Activity" means more than just drawing, so... (Or actually both should control it?)
	}
#endif

	SFML_WINDOW().display();
}


//----------------------------------------------------------------------------
void OONApp_sfml::event_loop()
{
	sf::Context context; //!! Seems redundant; it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details

	std::unique_lock noproc_lock{sync::Updating, std::defer_lock};

try {

	// Start the control panel unfocused:
	gui.unfocus();

	while (!terminated() && SFML_WINDOW().isOpen()) {

		// We could set up our own input poller directly using SAL::event::Source,
		// but since we already have a UI handy, doing the same, we can just get
		// the inputs from that. It will also already filter out the useless
		// MouseMovedRaw spam messages for us.
		//
		// Just make sure not to disable the control panel (a.k.a. `gui`)! :)
		// (Hiding (e.g. not drawing) it is fine.)
		//
		for (myco::event::Input event; !terminated() && (event = gui.poll(false));) {
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

//LOGD << "- acquiring lock for events...";
			noproc_lock.lock();
#endif
			if (!SFML_WINDOW().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
				LOGE << "\n- [event_loop] sf::setActive(false) failed!";
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

			UI::update_keys_from_Myco(event); //! Using the SFML adapter (via UI/adapter/SFML/...)
				//!! This should be generalized beyond keys, and should also make it possible
				//!! to use abstracted event types/codes for dispatching (below)!

//!! It felt more uneven if done here (due to the too coarse thread granularity of Windows -- and/or my own botched threading logic)! :-o
//!!			get_control_inputs(); // Should follow update_keys_from_SFML() (or else they'd get out of sync by some thread-switching delay!), until that's ensured implicitly!

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses" // F*, I'm not willing to litter the already noisy conditions below with even more parens...
#endif

			// Close req.?
			if (event.type == myco::event::WindowClosed ||
			    event.type == myco::event::KeyDown && event.get_if<myco::event::KeyDown>()->code == SFML_KEY(Escape)) { //!!XLAT

				if (gui.focused()) {
					gui.unfocus();
					//!! FTR:
					//!! A `break;` here would crash with a runtime_error caught at line 654 with the
					//!! mysterious message "resource deadlock would occur", *AND* THEN ALSO A CRASH!... :-o
					//!! (But that "extra" crash might be due to the shitty WinDbg failing to start a debug session.
					//!! Not that it would be normal for it to be triggered at all to begin with, if the exception
					//!! was already caught!... :-o )
					//!! -> #sfw2/486
				} else {
					request_exit();
					// [fix-setactive-fail] -> DON'T: window.close();
					//!!?? I forgot: how exactly is the window being closed on Esc?
					//!!?? I *guess* by the sf::Window dtor, but why do I vaguely
					//!!?? recall endless annoying problems with that from earlier?!
					break;
				}
			}

			// If the GUI has the input focus, let it process the event
			// -- except for some that really don't belong there:
			if (gui.focused() &&
			    (
				event.type != myco::event::WindowFocused && // Yeah, so this is an entirely different "focus"! :-o
				event.type != myco::event::WindowUnfocused &&
				event.type != myco::event::MouseButtonDown
				||
				event.type == myco::event::MouseButtonDown && gui.hovered()
				||
				event.type == myco::event::KeyDown
			    )
			)
			{
				goto process_ui_event;
			}
			// Else:
			gui.unfocus(); // A bit hamfisted, but: the event is ours, let the UI know!...

			//!! There's no sane way currently (for tha lack of a command/action queue)
			//!! to distinguish between player and non-player actions yet... Also, there's
			//!! even less about *which* player it is!... :)
			player_mark_active(/*!!Also no support for multiple players...!!*/);

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

			switch (event.type) //!! See above: morph into using abstracted events!
			{
			case myco::event::KeyDown:
			{
				auto keycode = event.get_if<myco::event::KeyDown>()->code;
#ifdef DEBUG
	if (cfg.DEBUG_show_keycode) Note("key code: " + keycode); //!! SFML3 has started making things harder every day... :-/
#endif
				switch (keycode) {
				case SFML_KEY(Pause): toggle_pause(); break;
				case SFML_KEY(Enter): time_step(1); break;
				case SFML_KEY(Backspace): time_step(-1); break;

				case SFML_KEY(Tab): toggle_interact_all(); break;

				case SFML_KEY(Insert): spawn(player_entity_ndx(),
						keystate(SHIFT) ? 100 : keystate(CTRL) ? 10 : 1); break;
				case SFML_KEY(Delete): remove_random_bodies(
						keystate(SHIFT) ? 100 : keystate(CTRL) ? 10 : 1); break;

//#586:				case SFML_KEY(F1):  keystate(SHIFT) ? quick_load_snapshot(1) : quick_save_snapshot(1); break;
				case SFML_KEY(F1): toggle_help(); break; // See also '?'!

				case SFML_KEY(F2):  keystate(SHIFT) ? quick_load_snapshot(2) : quick_save_snapshot(2); break;
				case SFML_KEY(F3):  keystate(SHIFT) ? quick_load_snapshot(3) : quick_save_snapshot(3); break;
				case SFML_KEY(F4):  keystate(SHIFT) ? quick_load_snapshot(4) : quick_save_snapshot(4); break;
				case SFML_KEY(F5):  keystate(SHIFT) ? quick_load_snapshot(5) : quick_save_snapshot(5); break;
				case SFML_KEY(F6):  keystate(SHIFT) ? quick_load_snapshot(6) : quick_save_snapshot(6); break;
				case SFML_KEY(F7):  keystate(SHIFT) ? quick_load_snapshot(7) : quick_save_snapshot(7); break;
				case SFML_KEY(F8):  keystate(SHIFT) ? quick_load_snapshot(8) : quick_save_snapshot(8); break;

				case SFML_KEY(Home): // See also Numpad5!
					if (keystate(CTRL)) {
						//!! These should be "upgraded" to "Camera/view reset"!
						//!! oon_main_camera().reset() already exists, but that's
						//!! not enough; see notes in zoom_reset() why!

						//!! Alas, pan_reset below also clears the focused entity.
						//!! It would be better to preserve it...
						//!!auto save_focused = focused_entity_ndx;

						pan_view_reset();
						//zoom_reset();

						//!!focused_entity_ndx = save_focused;
						//!! ...but a bad side-effect of that currently is implicit
						//!! automatic view-confinement -- immediately messing up
						//!! the view position if the focus object is bolting away! :)
						//!! Would be better to keep the focus obj. and just turn
						//!! off view confinement, but it can't be done yet. :-/
					} else {
 						// Select the player obj. by default (or with a dedicated modifier); same as with MouseButton!
						if (/*keystate(ALT) || */focused_entity_ndx == Entity::NONE)
							focused_entity_ndx = player_entity_ndx();

						assert(focused_entity_ndx != Entity::NONE);
						center(focused_entity_ndx);
					}
					break;

				case SFML_KEY(Numpad5): // See also Ctrl+Home!
					pan_view_reset();
					zoom_reset();
					break;

				case SFML_KEY(F12): toggle_huds();
					myco::set<myco::CheckBox>("Show HUDs", huds_active());
					break;
				case SFML_KEY(F11):
					toggle_fullscreen();
					//!! Refresh all our own (app-level) dimensions, too!
					//!! E.g. #288, and wrong .view size etc.!...
					break;

				default: // Keep GCC happy about unhandled enum values...
//LOGD << "UNHANDLED KEYPRESS: " << event.key.code;
					;
				}
				break;
			}
			case myco::event::TextInput:
			{
				const auto* textinput = event.get_if<myco::event::TextInput>();
				if (textinput->codepoint > 127) break; // non-ASCII!
				switch (static_cast<char>(textinput->codepoint)) {
				case 'g':
					myco::call<GravityModeSelector>("Gravity mode",
						[](auto* gs) { gs->selectNext(); });
					break;
//				case 'f': world().friction -= 0.01f; break;
//				case 'F': world().friction += 0.01f; break;
				case 'r': time.reversed = !time.reversed; break;
				case 't': time.scale *= 2.0f; break;
				case 'T': time.scale /= 2.0f; break;
				case 'h': toggle_pause(); break;
				case 'm': toggle_muting();
					myco::set<myco::CheckBox>("Audio: ", backend.audio.enabled);
					break;
				case 'M': toggle_music();
					myco::set<myco::CheckBox>(" - Music: ", backend.audio.music_enabled);
					break;
				case 'N': toggle_sound_fx();
					myco::set<myco::CheckBox>(" - FX: ", backend.audio.fx_enabled);
					break;
//!! #543			case 'P': fps_throttling(!fps_throttling()); break;
				case 'x': toggle_fixed_model_dt();
					myco::set<myco::CheckBox>("Fixed model Δt", cfg.fixed_model_dt_enabled);
					break;
				case '?': toggle_help(); break; // See also F1!
				}
				break;
/*!!NOT YET, AND NOT FOR SPAWN (#83):
			case myco::event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					spawn(player_entity_ndx(), 100);
				}
				break;
!!*/
			}
			case myco::event::MouseWheel:
			{
				//!! As a quick workaround for #334, we just check the GUI rect here
				//!! directly and pass the event if it belongs there...
//sf::Vector2f mouse = gui.mouse_position() + gui.getPosition();
//LOGD << "-- mouse: " << mouse.x <<", "<< mouse.y;
				if (gui.focused() || gui.hovered())
					goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				auto mousewheel = event.get_if<myco::event::MouseWheel>();
				view_control(mousewheel->delta); //! Apparently always 1 or -1...
//oon_main_view().p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4;
				break;
			}

			case myco::event::MouseButtonDown:
			{
				const auto* mousepress = event.get_if<myco::event::MouseButtonDown>();
//sf::Vector2f mouse = gui.mouse_position() + gui.getPosition();
//LOGD << "-- mouse: " << event.mouseButton.x <<", "<< event.mouseButton.y;

//!!??auto vpos = oon_main_camera().screen_to_view_coord(x, y); //!!?? How the FUCK did this compile?!?!? :-o
//!!?? Where did this x,y=={-520,-391} come from?! :-ooo
//!!??cerr << "???? x = " << x << ", y = " << y << " <-- WHAT THE HELL ARE THESE??? :-ooo\n";

				//!! As a quick workaround for #334, we just check the GUI rect here
				//!! directly and pass the event if it belongs there...
				if (gui.hovered())
					goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				auto vpos = oon_main_camera().screen_to_view_coord(mousepress->position.x, mousepress->position.y);
				oon_main_camera().focus_offset = vpos;
				EntityID clicked_entity_id = Entity::NONE;
				if (entity_at_viewpos(vpos.x, vpos.y, &clicked_entity_id)) {
LOGI << "Click: following object #"<<clicked_entity_id<<" now...";
				} else {
LOGD << "Click: no obj.";
					assert(clicked_entity_id == Entity::NONE);
				}

			//!! PROCESSING SHIFT MAKES NO SENSE WHILE ALSO HAVING SHIFT+MOVE, AS THAT WOULD ALWAYS JUST KEEP
			//!! THE CURRENT OBJECT AT THE M. POINTER, MAKING IT IMPOSSIBLE TO CLICK ON ANYTHING ELSE! :)
			//!! -- EVEN IF NOTHING IS SELECTED, AS SHIFT+MOVE IS FREE PANNING!...

				// Select the clicked object, if any (unless holding CTRL!)
				/*if (!keystate(CTRL))*/ //!! Really should be ALT, but... that's the stupid shield. :)
					focused_entity_ndx = clicked_entity_id == Entity::NONE
					                     ? (/*keystate(ALT) ? player_entity_ndx() // Select the player with a dedicated modifier; same as with Home!
				                                                : */(keystate(SHIFT) ? focused_entity_ndx : Entity::NONE))
				                             : clicked_entity_id; // Entity::NONE if none... //!!... Whoa! :-o See updates_for_next_frame()!
/*!!
				// Pan the selected object to focus, if holding SHIFT
				//!!?? -- WHAT? There should be no panning whatsoever on a simple click!
				if (keystate(SHIFT)) {
 					// Select the player by default; same as with Home!
 					// (Unless, as above, holding CTRL!)
					if (//!keystate(CTRL) &&
					    focused_entity_ndx == Entity::NONE)
						focused_entity_ndx = player_entity_ndx();
//!!?? -- SHIFT should just have the usual effect of locking the scroll!
					pan_to_focus(focused_entity_ndx); //! Tolerates Entity::NONE!
				}
!!*/
				if (focused_entity_ndx == Entity::NONE)
					Note("- Nothing there. Focusing on the deep void..."); //!! Do something better than this... :)
				break;
			}

			case myco::event::MouseMoved:
			{
				const auto* mousemove = event.get_if<myco::event::MouseMoved>();

				if (gui.focused() || gui.hovered()) goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				auto vpos = oon_main_camera().screen_to_view_coord(mousemove->position.x, mousemove->position.y);

				if (keystate(SHIFT) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) { //!! Direct SFML use!
					// pan_to_focus(anything), essentially:
					oon_main_camera().pan_view(oon_main_camera().focus_offset - vpos);
					oon_main_camera().focus_offset = vpos;
				}

				auto entity = Entity::NONE;
				if (entity_at_viewpos(vpos.x, vpos.y, &entity)) {
					hovered_entity_ndx = entity;
//LOGD << "Hover: pointing to #"<<hovered_entity_ndx;
				} else {
//LOGD << "Hover: no obj.";
					hovered_entity_ndx = Entity::NONE;
				}
				break;
			}

			case myco::event::WindowUnfocused:
				oon_main_view().dim();
				reset_keys(); //!! Should be an engine-internal chore...
				break;

			case myco::event::WindowFocused:
				oon_main_view().undim();
				reset_keys(); //!! Should be an engine-internal chore...
				break;

			default:
process_ui_event:		// The GUI should be given a chance before this `switch`, but... -> #334: it can't swallow events!
				gui.dispatch(event);
				//!! Also, it's kinda inconsistent with this `IDLE` state assumption below!...
				//!! (Hopefully it's not even used nowadays at all though...)
				ui_event_state = UIEventState::IDLE;

				break;
			} // switch

//LOGD << "sf::Context [event loop]: " << sf::Context::getActiveContextId();

			ui_event_state = UIEventState::EVENT_READY;

#ifdef DISABLE_THREADS
		} // for - events in the queue

		update_thread_main_loop(); // <- Doesn't actually loop, when threads are disabled, so crank it from here!
#else
//LOGD << "- freeing the proc. lock...";
			noproc_lock.unlock();

			//! Sleep some here, too (not just outside this inner loop, while
			//! idling), counterintuitively *especially* while jamming, in order to
			//! give the actual processing parts some unlocked time!...
			sf::sleep(sf::milliseconds(5)); //!! Should be adaptive! //!! DIRECT SFML USE
		} // for - events in the queue

		// The event queue has been emptied, so in this thread (of input processing)
		// we're idling now (while updates are happening elsewhere), so:
		//!! Should be adaptive!!
		sf::sleep(sf::milliseconds( //!! DIRECT SFML USE
			paused() ? cfg.get("sim/timing/paused_sleep_time_per_cycle", 50) // #330
			         : 10)); // SFML does (used to?) sleep the same amount for waitEvent
			// NOTE: This is only relevant when threading!
			// The non-threaded main loop sleeps via backend.hci.fps_throttling.
			//!!?? How about fps throttling *with* threading then?! -> #217

		if (paused() && ui_event_state == UIEventState::IDLE)
		{
			sf::sleep(sf::milliseconds(100)); // Sleep even more, if *really* idle! :) //!! DIRECT SFML USE
		}

#endif			

//!!IPROF_SYNC_THREAD;

	} // while - still running

} catch (runtime_error& x) {
	Error(x.what());
	return;
} catch (exception& x) {
	Error("EXCEPTION: "s + x.what());
	return;
} catch (...) {
	Error("UNKNOWN EXCEPTION!");
	return;
}
}


} // namespace OON
