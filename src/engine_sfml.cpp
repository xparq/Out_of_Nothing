#include "engine_sfml.hpp"

#include <SFML/System/Sleep.hpp>
//!!move to rendering:
#include <SFML/Graphics/CircleShape.hpp>

#include <memory> // make_shared
#include <iostream> // cerr

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
	}

	if (!window.setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [update_thread_main_loop] sf::setActive(false) failed!\n";
		terminate();
		return;
	}

	//! If there's still time left from the frame slice:
	sf::sleep(sf::milliseconds(100)); //!! (remaining_time_ms)
		//!!This doesn't seem to have any effect on the CPU load! :-o
		//!!Nor does it ruin the smooth rendering! :-o WTF?!
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
		hud.draw(window);
	}
#endif
	window.display();
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
/*!!?? Why does this one (always) fail?!
		if (!window.pollEvent(event)) {
			cerr << "- Event processing failed. WTF?! Terminating.\n";
			exit(-1);
		}
??!!*/			
		for (; window.pollEvent(event);) {
#endif					

			//!! The update thread may be busy calculating, so we can't just go ahead and change things!...
			//!!while (busy_calculating())
			//!!	;

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
				case '+': zoom_in(); break;
				case '-': zoom_out(); break;
				case 'o': pan_reset(); break;
				case 'h': pan_center_body(0); break;
				case ' ': toggle_pause(); break;
				case 'm': toggle_music(); break;
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
cerr << "BEGIN sf::Event::Closed\n";
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
//!!			draw();
#endif			
	} // while
}

//----------------------------------------------------------------------------
auto Engine_SFML::add_body(World_SFML::Body&& obj)
{
	auto ndx = world.add_body(std::forward<decltype(obj)>(obj));

	// For rendering...

	//! Not all the Drawables are also Transformables! (E.g. vertex arrays.)
	// (But our little ugly circles are, for now.)
	auto shape = make_shared<sf::CircleShape>(obj.r * _SCALE);
	renderer.shapes_to_draw.push_back(shape);
	renderer.shapes_to_change.push_back(shape); // "to transform"

	return ndx;
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

	clack_sound = audio.add_sound("resource/sound/clack.wav");

	audio.play_music("resource/music/default.ogg");

#ifdef HUD_ENABLED
	_setup_huds();
#endif	
}

#ifdef HUD_ENABLED	
void Engine_SFML::_setup_huds()
{
	static auto& s_OFFSET_X = _OFFSET_X;
	static auto& s_OFFSET_Y = _OFFSET_Y;
	static auto& s_SCALE = _SCALE;
	static auto& s_G = world.G;
	static auto& s_globe_x = world.bodies[globe_ndx]->p.x;
	static auto& s_globe_y = world.bodies[globe_ndx]->p.y;
	static auto& s_globe_vx = world.bodies[globe_ndx]->v.x;
	static auto& s_globe_vy = world.bodies[globe_ndx]->v.y;

//		hud.add("FPS", [this]()->string { return to_string(1000 / this->world.dt); });
	hud.add("pan X", &s_OFFSET_X);
	hud.add("pan Y", &s_OFFSET_Y);
	hud.add("SCALE", &s_SCALE);
	hud.add("globe mass", &world.bodies[globe_ndx]->mass);
	hud.add("globe x", &s_globe_x);
	hud.add("globe y", &s_globe_y);
	hud.add("globe vx", &s_globe_x);
	hud.add("globe vy", &s_globe_y);
}
#endif
