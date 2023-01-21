#include "engine_sfml.hpp"

#include <SFML/System/Sleep.hpp>

#include <memory>
	using std::make_shared;
#include <iostream>
	using std::cerr, std::endl;
#include <cstdlib>
	using std::rand; // and the RAND_MAX macro!

using namespace std;

//----------------------------------------------------------------------------
void Engine_SFML::update_thread_main_loop()
{
	while (!terminated()) {
		switch (ui_event_state) {
		case UIEventState::BUSY:
//cerr << " [[[...BUSY...]]] ";
			break;
		case UIEventState::IDLE:
		case UIEventState::EVENT_READY:
			updates_for_next_frame();
			draw();
			break;
		default:
cerr << " [[[...!!??UNKNOWN EVENT STATE??!!...]]] ";
		}

	//! If there's still time left from the frame slice:
	sf::sleep(sf::milliseconds(30)); //!! (remaining_time_ms)
		//!!This doesn't seem to have any effect on the CPU load! :-o
		//!!Nor does it ruin the smooth rendering! :-o WTF?!
	}
/*
	if (!window.setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [update_thread_main_loop] sf::setActive(false) failed!\n";
		terminate();
		return;
	}
*/
}

//----------------------------------------------------------------------------
void Engine_SFML::draw()
{
	renderer.render(*this);
		// Was in updates_for_next_frame(), but pause should not stop UI updates.
		//!!...raising the question: at which point should UI rendering be separated from world rendering?

	window.clear();

	renderer.draw(*this);
#ifdef HUD_ENABLED
	if (_show_huds) {
		debug_hud.draw(window);
		if (_show_help) help_hud.draw(window);
	}
#endif
	window.display();
}

//----------------------------------------------------------------------------
void Engine_SFML::updates_for_next_frame()
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	if (paused()) return;

	world.recalc_for_next_frame(*this);
}


//----------------------------------------------------------------------------
void Engine_SFML::event_loop()
{
	while (window.isOpen() && !terminated()) {
			sf::Event event;

#ifdef THREADS_ENABLED
		if (!window.waitEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}
#else
/*!!?? Why did this (always?) fail?!
		if (!window.pollEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}
??!!*/			
		for (; window.pollEvent(event);) {
#endif					
			//!! The update thread may still be busy calculating, so we can't just go ahead and change things!
			//!! But... then again, how come this thing still works at all?! :-o
			//!! Clearly there must be cases when processing the new event here overlaps with an ongoing update
			//!! cycle before they notice our "BUSY" state change here, and stop! :-o
			//!! So... a semaphore from their end must be provided to this point of this thread, too!
			//!! And it may not be just as easy as sg. like:
			//!!while (engine.busy_updating())
			//!!	;
			//!! A much cleaner way would be pushing the events received here
			//!! into a queue in the update/processing thread, like:
			//!! engine.inputs.push(event);
			//!! (And then the push here and the pop there must be synchronized -- hopefully just <atomic> would do.)

			ui_event_state = UIEventState::BUSY;

			switch (event.type)
			{
			case sf::Event::KeyReleased:
				switch (event.key.code) {
				case sf::Keyboard::Up:    up_thruster_stop(); break;
				case sf::Keyboard::Down:  down_thruster_stop(); break;
				case sf::Keyboard::Left:  left_thruster_stop(); break;
				case sf::Keyboard::Right: right_thruster_stop(); break;
				}
				break;

			case sf::Event::KeyPressed:
				switch (event.key.code) {
				case sf::Keyboard::Escape: //!!Merge with Closed!
					terminate();
					// [fix-setactive-fail] -> DON'T: window.close();
					break;

				case sf::Keyboard::Up:
					if (event.key.shift) pan_up();
					else                 up_thruster_start();
					break;
				case sf::Keyboard::Down:
					if (event.key.shift) pan_down();
					else                 down_thruster_start();
					break;
				case sf::Keyboard::Left:
					if (event.key.shift) pan_left();
					else                 left_thruster_start();
					break;
				case sf::Keyboard::Right:
					if (event.key.shift) pan_right();
					else                 right_thruster_start();
					break;

				case sf::Keyboard::F12: toggle_huds(); break;
				}
				break;

			case sf::Event::MouseWheelScrolled:
				renderer.p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4; //! seems to always be 1 or -1...
				break;

			case sf::Event::TextEntered:
				if (event.text.unicode > 128) break; // non-ASCII!
				switch (static_cast<char>(event.text.unicode)) {
				case 'n': add_body(); break;
				case 'd': remove_body(); break;
				case '+': zoom_in(); break;
				case '-': zoom_out(); break;
				case 'o': pan_reset(); break;
				case 'h': pan_center_body(0); break;
				case ' ': toggle_pause(); break;
				case 'm': toggle_music(); break;
				case '?': toggle_help(); break;
				}
				break;

			case sf::Event::LostFocus:
				renderer.p_alpha = Renderer_SFML::ALPHA_INACTIVE;
				break;

			case sf::Event::GainedFocus:
				renderer.p_alpha = Renderer_SFML::ALPHA_ACTIVE;
				break;

			case sf::Event::Closed: //!!Merge with key:Esc!
				terminate();
cerr << "BEGIN sf::Event::Closed\n"; //!!this frame is to trace an error from the lib/OpenGL
				window.close();
cerr << "END sf::Event::Closed\n";
				break;

			default:

				ui_event_state = UIEventState::IDLE;

				break;
			}

			ui_event_state = UIEventState::EVENT_READY;

#ifndef THREADS_ENABLED
		} // for

		updates_for_next_frame();

		draw();
//!!test idempotency:
//!!	draw();
#endif			
	} // while
}

//----------------------------------------------------------------------------
size_t Engine_SFML::add_body(World_SFML::Body&& obj)
{
	auto ndx = world.add_body(std::forward<decltype(obj)>(obj));
	// Pre-cache shapes for rendering... (!! Likely pointless, but this is just what I started with...)
	renderer.create_cached_body_shape(*this, obj, ndx);
	return ndx;
}

size_t Engine_SFML::add_body()
{
	auto r_min = CFG_GLOBE_RADIUS / 10;
	auto r_max = CFG_GLOBE_RADIUS * 0.5;
	auto p_range = CFG_GLOBE_RADIUS * 5;
	auto v_range = CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!

cerr << "Adding new object #" << world.bodies.size() + 1 << "...\n";

//!!atrocious hack to wait for the ongoing update to finish!... ;)
//!!test whether std::atomic could solve it!
sf::sleep(sf::milliseconds(1));
//!!Doesn't seem to crash regardless!... :-o :)

	return add_body({
		.r = (float) ((rand() * (r_max - r_min)) / RAND_MAX ) //! suppress warning "conversion from double to float..."
				+ r_min,
		.p = { (rand() * p_range) / RAND_MAX - p_range/2,
		       (rand() * p_range) / RAND_MAX - p_range/2 },
		.v = { (rand() * v_range) / RAND_MAX - v_range/2,
		       (rand() * v_range) / RAND_MAX - v_range/2 },
		.color = (uint32_t) (float)0xffffff * rand(),
	});
}

void Engine_SFML::remove_body(size_t ndx)
{
	world.remove_body(ndx);
	renderer.delete_cached_body_shape(*this, ndx);
}

void Engine_SFML::remove_body()
{
	if (world.bodies.size() < 2) { // Leave the "globe" (so not ".empty()")!
cerr <<	"No more \"free\" items to delete.\n";
		return;
	}

//!!atrocious hack to wait for the ongoing update to finish!... ;)
//!!test whether std::atomic could solve it!
sf::sleep(sf::milliseconds(1)); // It DOES crash without this! ;)

	auto ndx = 1/*leave the globe!*/ + rand() * ((world.bodies.size()-1) / (RAND_MAX + 1));
cerr << "Deleting object #"	 << ndx << "...\n";
	assert(ndx > 0);
	assert(ndx < world.bodies.size());
	remove_body(ndx);
}


//----------------------------------------------------------------------------
void Engine_SFML::_setup()
{
	// globe:
	globe_ndx = add_body({ .r = CFG_GLOBE_RADIUS, .density = world.DENSITY_ROCK, .p = {0,0}, .v = {0,0}, .color = 0xb02000});
	// moons:
	add_body({ .r = CFG_GLOBE_RADIUS/10, .p = {CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -CFG_GLOBE_RADIUS * 2}, .color = 0x14b0c0});
	add_body({ .r = CFG_GLOBE_RADIUS/7,  .p = {-CFG_GLOBE_RADIUS * 1.6f, +CFG_GLOBE_RADIUS * 1.2f}, .v = {-CFG_GLOBE_RADIUS*1.8, -CFG_GLOBE_RADIUS*1.5},
				.color = 0xa0f000});

	clack_sound = audio.add_sound("asset/sound/clack.wav");

	audio.play_music("asset/music/default.ogg");

#ifdef HUD_ENABLED
	_setup_huds();
#endif	
}

#ifdef HUD_ENABLED	
void Engine_SFML::_setup_huds()
{
	//!!?? Why do all these member pointers just work, also without so much as a warning,
	//!!?? in this generic pointer passing context?!
	debug_hud.add("Press ? for help...");
	debug_hud.add("frame delay (s)", &world.dt);
//!!debug_hud.add("FPS", [this]()->string { return to_string(1 / this->world.dt); });
//	debug_hud.add("pan X", &_OFFSET_X);
//	debug_hud.add("pan Y", &_OFFSET_Y);
//	debug_hud.add("SCALE", &_SCALE);

//	debug_hud.add("globe R", &CFG_GLOBE_RADIUS); //!!now this one does crash! :-o
	debug_hud.add("globe R", &world.bodies[globe_ndx]->r); //!!and also this did earlier! :-o WTF??!?! how come ->mass didn't then?!?!
	                                                       //!!??and how come it doesn't again after a recompilation?!?!?!?!
	debug_hud.add("globe m", &world.bodies[globe_ndx]->mass);
	debug_hud.add("globe x",    &world.bodies[globe_ndx]->p.x);
	debug_hud.add("globe y",    &world.bodies[globe_ndx]->p.y);
	debug_hud.add("globe vx",   &world.bodies[globe_ndx]->v.x);
	debug_hud.add("globe vy",   &world.bodies[globe_ndx]->v.y);

	help_hud.add("THIS IS NOT A TOY. DO NOT SWALLOW.");
	help_hud.add("");
	help_hud.add("F12: toggle HUDs");
	help_hud.add("arrows: thrust");
	help_hud.add("n: add new object");
	help_hud.add("d: delete (randomly selected) object");
	help_hud.add("Space: pause (physics)");
	help_hud.add("h: home in on the globe");
	help_hud.add("o: reset pan offset");
	help_hud.add("+/-: zoom");
	help_hud.add("Shift+arrows: pan");
	help_hud.add("m: toggle music");
	help_hud.add("mouse wheel: alpha fading (for debugging)");
	help_hud.add("Esc: quit");
}
#endif
