#include "OON_sfml.hpp"
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

using namespace Model;
using namespace UI;
using namespace std;

//
// "App-local global" implementation-level (const) params...
//
static constexpr auto WINDOW_TITLE = "Out of Nothing";
namespace cfg {
	static constexpr auto FPS_THROTTLE = 30; //!! Changing this changes the physics (by increasing resolution/precision)!!!
	                                         //!! Things tend to be more interesting, with more "quantum-like" randomness,
											 //!! with larger dt-s (less precision -> overshoots, tunnelling!)!
}

namespace sync {
	std::mutex Updating;
};


//============================================================================
OON_sfml::OON_sfml()
		// Creating the window right away here (only) to support init-by-constr. for the HUDs:
		: window(sf::VideoMode({Renderer_SFML::WINDOW_WIDTH, Renderer_SFML::WINDOW_HEIGHT}), WINDOW_TITLE)
		//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
		//!!??
		//sf::glEnable(sf::GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?!
		//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0

#ifndef DISABLE_HUD
			, debug_hud(window, -220)
			, help_hud(window, 10, HUD::DEFAULT_PANEL_TOP, 0x40d040ff, 0x40f040ff/4) // left = 10
#endif
{
		_setup();
}

bool OON_sfml::run()
{
	//! The event loop will block and sleep.
	//! The update thread is safe to start before the event loop, but we should also draw something
	//! already before the first event, so we have to release the SFML (OpenGL) Window (crucial!),
	//! and unfreeze the update thread (which would wait on the first event by default).
	if (!window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(false) failed, WTF?! Terminating.\n";
		return false;
	}

	ui_event_state = SimApp::UIEventState::IDLE;

#ifndef DISABLE_THREADS
	std::thread game_state_updates(&OON_sfml::update_thread_main_loop, this);
		//! NOTES:
		//! - When it wasn't a member fn, the value vs ref. form was ambiguous and failed to compile!
		//! - The thread ctor would *copy* its params (by default), which would be kinda wonky for the entire app. ;)
#endif

	event_loop();

#ifndef DISABLE_THREADS
//cerr << "TRACE - before threads join\n";
	game_state_updates.join();
#endif

	return true;
}

//----------------------------------------------------------------------------
void OON_sfml::update_thread_main_loop()
{
	sf::Context context; //!! Seems redundant, as it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details
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
			if (!window.setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
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
			if (!window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
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
void OON_sfml::draw()
{
	renderer.render(*this);
		// Was in updates_for_next_frame(), but pause should not stop UI updates.
		//!!...raising the question: at which point should UI rendering be separated from world rendering?

	window.clear();

	renderer.draw(*this);
#ifndef DISABLE_HUD
	if (_show_huds) {
		debug_hud.draw(window);
		if (help_hud.active()) help_hud.draw(window); //!! the active-chk is redundant, the HUD does the same; TBD, who's boss!
		                                              //!! "activity" may mean more than drawing. so... actually both can do it?
	}
#endif

	window.display();
}

//----------------------------------------------------------------------------
void OON_sfml::updates_for_next_frame()
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	if (physics_paused()) {
		sf::sleep(sf::milliseconds(50)); //!!that direct 50 is gross, but...
		return;
	}

	// In addition to the async. event loop:
	poll_and_process_controls();

	auto frame_delay = clock.getElapsedTime().asSeconds();
	clock.restart(); //! Must also be duly restarted on unpausing!
	avg_frame_delay.update(frame_delay);

	world.recalc_next_state(frame_delay, *this);

	// Auto-scroll to follow player movement...
	//!
	//! NOTE: THIS MUST COME AFTER RECALCULATING THE NEW STATE!
	//!
	if (keystate(SCROLL_LOCKED) || keystate(SHIFT)) {
		pan_to_player();
	}
}

//----------------------------------------------------------------------------
void OON_sfml::event_loop()
{
	sf::Context context; //!! Seems redundant; it can draw all right, but https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Context.php#details

	std::unique_lock noproc_lock{sync::Updating, std::defer_lock};

	while (window.isOpen() && !terminated()) {
			sf::Event event;

#ifndef DISABLE_THREADS
		if (!window.waitEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}

		ui_event_state = UIEventState::BUSY;
//cerr << "- acquiring lock for events...\n";
		noproc_lock.lock();

#else
/*!!?? Why did this (always?) fail?!
		if (!window.pollEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}
??!!*/			

		// This inner loop is only for non-threading mode, to prevent event processing
		// (reaction) delays (or even loss) due to accumulating events coming faster
		// than 1/frame (for a long enough period to cause noticable jam/stutter)!
		for (; window.pollEvent(event);) {

		ui_event_state = UIEventState::BUSY;
//cerr << "- acquiring events...\n";
#endif
			if (!window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
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
extern bool DEBUG_cfg_show_keycode; if (DEBUG_cfg_show_keycode) cerr << "key code: " << event.key.code << "\n";
#endif

				switch (event.key.code) {
				case sf::Keyboard::Escape: //!!Merge with Closed!
					terminate();
					// [fix-setactive-fail] -> DON'T: window.close();
					break;

				case sf::Keyboard::Pause: toggle_pause_physics(); break;
				case sf::Keyboard::Tab: toggle_interact_all(); break;

				case sf::Keyboard::Insert: spawn(keystate(SHIFT) ? 1 : 100); break;
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

				case sf::Keyboard::F12: toggle_huds(); break;
				case sf::Keyboard::F11: toggle_fullscreen(); break;

//				default:
//cerr << "UNHANDLED KEYPRESS: " << event.key.code << endl;
				}
				break;

			case sf::Event::TextEntered:
				if (event.text.unicode > 128) break; // non-ASCII!
				switch (static_cast<char>(event.text.unicode)) {
				case 'f': world.FRICTION -= 0.01f; break;
				case 'F': world.FRICTION += 0.01f; break;
				case '+': zoom_in(); break;
				case '-': zoom_out(); break;
				case 'm': toggle_music(); break;
				case 'P': sw_fps_throttling(!sw_fps_throttling()); break;
				case 'M': toggle_sound_fxs(); break;
				case '?': toggle_help(); break;
				}
				break;
/*!!NOT YET, AND NOT FOR SPAWN (#83):
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					spawn(100);
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
				window.close();
//cerr << "END sf::Event::Closed\n";
				break;

			default:
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

//----------------------------------------------------------------------------
size_t OON_sfml::add_player(World::Body&& obj)
{
	// These are the player modelling differences:
	obj.add_thrusters();
	obj.superpower.gravity_immunity = true;
	obj.superpower.free_color = true;
	obj/*.superpower*/.lifetime = World::Body::Unlimited; //!!?? Should this be a superpower instead?

	return add_body(std::forward<World::Body>(obj));
}

void OON_sfml::remove_player(size_t ndx)
{ndx;
}


//----------------------------------------------------------------------------
bool OON_sfml::touch_hook(World* w, World::Body* obj1, World::Body* obj2)
{w;
	if (obj1->is_player() || obj2->is_player()) {
		audio.play_sound(clack_sound);
	}

	obj1->T += 100;
	obj2->T += 100;

	obj1->recalc();
	obj2->recalc();

	return false; //!!Not yet used!
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

	if (abs(vpos.x) > Renderer_SFML::VIEWPORT_WIDTH/2  - visible_R ||
	    abs(vpos.y) > Renderer_SFML::VIEWPORT_HEIGHT/2 - visible_R)
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
void OON_sfml::toggle_fullscreen()
{
	static bool is_full = false;

	//! NOTE: We're being called here from a locked section of the event loop.

	if (!window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
cerr << "\n- [toggle_fullscreen] sf::setActive(false) failed!\n";
	}

	is_full = !is_full;

	window.create(
		is_full ? sf::VideoMode::getDesktopMode() : sf::VideoMode({Renderer_SFML::WINDOW_WIDTH, Renderer_SFML::WINDOW_HEIGHT}),
		WINDOW_TITLE,
		is_full ? sf::Style::Fullscreen|sf::Style::Resize : sf::Style::Resize
	);
	sw_fps_throttling(sw_fps_throttling()); // awkward way to reactivate the last thr. state

	onResize();

	if (!window.setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
cerr << "\n- [toggle_fullscreen] sf::setActive(true) failed!\n";
	}

//	if (!(is_full = !is_full) /* :) */) {
//		// full
//	} else {
//		// windowed
//	}
}

//----------------------------------------------------------------------------
bool OON_sfml::sw_fps_throttling(int newstate/* = -1*/)
{
	static bool state = true;

	if (newstate != -1) {
		state = newstate;
		window.setFramerateLimit(state ? cfg::FPS_THROTTLE : 0);
	}
	return state;
}

void OON_sfml::onResize()
{
	debug_hud.onResize(window);
	help_hud.onResize(window);
}


//----------------------------------------------------------------------------
void OON_sfml::_setup()
{
	//! Note: the window itself has just been created by the ctor.!
	//! But... it will also be recreated each time the fullscreen/windowed
	//! mode is toggled, so this will need to be repeated after every
	//! `window.create` call (i.e. in `toggle_fullscreen`):
	sw_fps_throttling(true);

	// Player Superglobe:
	globe_ndx = add_player({.r = world.CFG_GLOBE_RADIUS, .density = Physics::DENSITY_ROCK, .p = {0,0}, .v = {0,0}, .color = 0xffff20});
	// moons:
	add_body({.r = world.CFG_GLOBE_RADIUS/10, .p = {world.CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -world.CFG_GLOBE_RADIUS * 2},
				.color = 0xff2020});
	add_body({.r = world.CFG_GLOBE_RADIUS/7,  .p = {-world.CFG_GLOBE_RADIUS * 1.6f, +world.CFG_GLOBE_RADIUS * 1.2f}, .v = {-world.CFG_GLOBE_RADIUS*1.8, -world.CFG_GLOBE_RADIUS*1.5},
				.color = 0x3060ff});

	clack_sound = audio.add_sound("asset/sound/clack.wav");

	audio.play_music("asset/music/default.ogg");
	/*
	static sf::Music m2; if (m2.openFromFile("asset/music/extra sonic layer.ogg")) {
		m2.setLoop(false); m2.play();
	}
	*/

	_setup_UI();
}

void OON_sfml::_setup_UI()
{
#ifndef DISABLE_HUD
	//!!?? Why do all these member pointers just work, also without so much as a warning,
	//!!?? in this generic pointer passing context?!
	//!!
	//!! "Evenfurthermore": why do all these insane `this` captures apparently survive
	//!! all the obj recreation shenanigans (they *are* recreated, right??...) after
	//!! a World reload?!?!?!
	//!!
	auto ftos = [this](auto* ptr_x) { return [this, ptr_x]() { static constexpr size_t LEN = 15;
		char buf[LEN + 1]; auto [ptr, ec] = std::to_chars(buf, buf+LEN, *ptr_x);
		return string(ec != std::errc() ? "???" : (*ptr = 0, buf));
		};
	};
	debug_hud.add("FPS",        [this](){ return to_string(1 / (float)this->avg_frame_delay); });
	debug_hud.add("# of objs.", [this](){ return to_string(this->world.bodies.size()); });
	debug_hud.add("Body interactions", &this->world._interact_all);
	debug_hud.add("Drag", ftos(&this->world.FRICTION));
	debug_hud.add("");
	debug_hud.add("Globe T",  ftos(&this->world.bodies[this->globe_ndx]->T));
	debug_hud.add("      R",  ftos(&this->world.bodies[this->globe_ndx]->r));
	debug_hud.add("      m",  ftos(&this->world.bodies[this->globe_ndx]->mass));
	debug_hud.add("      x",  ftos(&this->world.bodies[this->globe_ndx]->p.x));
	debug_hud.add("      y",  ftos(&this->world.bodies[this->globe_ndx]->p.y));
	debug_hud.add("      vx", ftos(&this->world.bodies[this->globe_ndx]->v.x));
	debug_hud.add("      vy", ftos(&this->world.bodies[this->globe_ndx]->v.y));
	debug_hud.add("");
	debug_hud.add("VIEW SCALE", &view.zoom);
	debug_hud.add("CAM. X", &view.offset.x);
	debug_hud.add("CAM. Y", &view.offset.y);
/*	debug_hud.add("");
	debug_hud.add("SHIFT", (bool*)&_kbd_state[SHIFT]);
	debug_hud.add("LSHIFT", (bool*)&_kbd_state[LSHIFT]);
	debug_hud.add("RSHIFT", (bool*)&_kbd_state[RSHIFT]);
	debug_hud.add("CAPS LOCK", (bool*)&_kbd_state[CAPS_LOCK]);
	debug_hud.add("SCROLL LOCK", (bool*)&_kbd_state[SCROLL_LOCK]);
	debug_hud.add("NUM LOCK", (bool*)&_kbd_state[NUM_LOCK]);
*/
	debug_hud.add("");
	debug_hud.add("Press ? for help...");

	//------------------------------------------------------------------------
	help_hud.add("------- Controls:");
	help_hud.add("Arrows: thrust");
	help_hud.add("Space:  exhaust trail");
	help_hud.add("Ins:    add 100 objects (+Shift: only 1)");
	help_hud.add("Del:    remove 100 objects (Shift+R: only 1)");
//	help_hud.add("------- Metaphysics:");
	help_hud.add("Tab:    toggle all-body interactions");
	help_hud.add("F:      decrease (+Shift: incr.) drag (friction)");
//	help_hud.add("C:      chg. collision mode: pass/stick/bounce");
	help_hud.add("Pause:  stop the physics");
	help_hud.add("mouse wheel (or +/-): zoom");
	help_hud.add("AWSD:   pan");
	help_hud.add("Shift:  autoscroll to follow player movement");
	help_hud.add("Scroll Lock: toggle autoscroll");
	help_hud.add("Home:   home in on the player globe");
	help_hud.add("Ctrl+Home: reset view to Home pos. (not the zoom)");
	help_hud.add("------- Meta:"); //!!Find another label, like "Console" or "Admin"...
	help_hud.add("F1-F4:  save world snapshots (+Shift: load)");
	help_hud.add("M:      (un)mute music (+Shift: same for the fx.)");
	help_hud.add("Shft+P: toggle FPS throttling (lower CPU load)");
	help_hud.add("F11:    toggle fullscreen");
	help_hud.add("F12:    toggle HUDs");
	help_hud.add("");
	help_hud.add("Esc:    quit");
	help_hud.add("");
	help_hud.add("Command-line options: oon.exe /?");

	help_hud.active(true);
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
/* A cleaner, but slower way would be this:
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
	for (size_t n = 0; n < world.bodies.size(); ++n) {
		renderer.create_cached_body_shape(*this, *world.bodies[n], n);
	}
cerr << "Game state restored from slot " << slot_id << ".\n";
	return true;
}
