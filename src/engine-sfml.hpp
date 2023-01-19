#ifndef __ENGINE_SFML__
#define __ENGINE_SFML__

#include "cfg.h"

#include "world-sfml.hpp"
#include "renderer-sfml.hpp"
#include "hud_sfml.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
//!!move to rendering:
#include <SFML/Graphics/CircleShape.hpp>

#include <memory> // shared_ptr
#include <atomic>
#include <vector>
#include <iostream> // cerr
using namespace std;


//----------------------------------------------------------------------------
class Engine // "Controller"
//----------------------------------------------------------------------------
{
// Config
public:
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!
	static constexpr float CFG_GLOBE_RADIUS = 50000000; // m
	static constexpr float CFG_THRUST_FORCE = 6e34; // N (kg*m/s^2)
	
	static constexpr float CFG_DEFAULT_SCALE = 0.000001; //! This one also depends very much on the physics!

	static constexpr float CFG_PAN_STEP = 10; // "SFML defaul pixel" :) (Not quite sure yet how it does coordinates...)

// Player-controls (state)
public:

	size_t globe_ndx = 0; // just a paranoid safety init (see _setup() tho!)

	struct Thruster {
		float _throttle = 0;
		float throttle(float new_throttle)
		{
			auto prev_throttle = _throttle;
			_throttle = new_throttle;
			return prev_throttle;
		}
		float throttle() const { return _throttle; }
	};
	//!!Thtusters should have vectorized throttles relative to the body orientation,
	//!!which is currently fixed to be identical to the world coordinate system...
	Thruster thrust_up;
	Thruster thrust_down;
	Thruster thrust_left;
	Thruster thrust_right;

public:
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

protected:
	bool _terminated = false;
	bool _paused = false;
	bool _show_huds = true;

public:
	auto toggle_pause()  { _paused = !_paused; pause(_paused); }
	auto paused()  { return _paused; }
	virtual void pause(bool state = true) = 0; //!! dumb way to depend on the actual World type...

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	auto toggle_huds()  { _show_huds = !_show_huds; }
};

//----------------------------------------------------------------------------
class Engine_SFML : public Engine
{
friend class Renderer_SFML;

// Internals... -- not quite yet; just allow access for now:
public:
	World_SFML  world;
	Renderer_SFML renderer;

	void pause(bool state = true)  override { _paused = state; world.pause(state); }

#ifdef HUD_ENABLED
	HUD_SFML    hud;
#endif

protected:
	float _SCALE = CFG_DEFAULT_SCALE;
	float _OFFSET_X = 0, _OFFSET_Y = 0;

public: //!!Currently used by the global standalone fn event_loop() directly!
	sf::RenderWindow& window;
//!!was:	sf::RenderWindow* window; // unique_ptr<sf::RenderWindow> window would add nothing but unwarranted complexity here
public:
// Ops
	//! Should be idempotent to tolerate keyboard repeats (which could be disabled, but better be robust)!
	auto up_thruster_start()    { thrust_up.throttle(CFG_THRUST_FORCE); }
	auto down_thruster_start()  { thrust_down.throttle(CFG_THRUST_FORCE); }
	auto left_thruster_start()  { thrust_left.throttle(CFG_THRUST_FORCE); }
	auto right_thruster_start() { thrust_right.throttle(CFG_THRUST_FORCE); }

	auto up_thruster_stop()     { thrust_up.throttle(0); }
	auto down_thruster_stop()   { thrust_down.throttle(0); }
	auto left_thruster_stop()   { thrust_left.throttle(0); }
	auto right_thruster_stop()  { thrust_right.throttle(0); }

	auto pan_up()     { _OFFSET_Y -= CFG_PAN_STEP; }
	auto pan_down()   { _OFFSET_Y += CFG_PAN_STEP; }
	auto pan_left()   { _OFFSET_X -= CFG_PAN_STEP; }
	auto pan_right()  { _OFFSET_X += CFG_PAN_STEP; }
	auto pan_reset()  { _OFFSET_X = _OFFSET_Y = 0; }
	auto pan_center_body(auto body_id) {
		const auto& body = world.bodies[body_id];
		_OFFSET_X = - body->p.x * _SCALE;
		_OFFSET_Y = - body->p.y * _SCALE;
	}
	auto _pan_adjust_after_zoom() {
		//!!??
	}

	auto zoom_in()  { auto factor = 1.25; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}
	auto zoom_out () { auto factor = 0.80; _SCALE *= factor;
		renderer.resize_objects(factor);
		_pan_adjust_after_zoom();
	}

	auto updates_for_next_frame()
	// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
	{
		if (paused()) return;

		world.recalc_for_next_frame(*this);
	}

	auto draw()
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

	//------------------------------------------------------------------------
	void update_thread_main_loop()
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

		//if there's still time:
		//sf::sleep(sf::milliseconds(remaining_time_ms));
	}

	//------------------------------------------------------------------------
	void event_loop()
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

	auto add_body(World_SFML::Body&& obj)
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

	auto _setup()
	{
		// globe:
		globe_ndx = add_body({ .r = CFG_GLOBE_RADIUS, .density = world.DENSITY_ROCK, .p = {0,0}, .v = {0,0}, .color = 0xb02000});
		// moons:
		add_body({ .r = CFG_GLOBE_RADIUS/10, .p = {CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -CFG_GLOBE_RADIUS * 2}, .color = 0x14b0c0});
		add_body({ .r = CFG_GLOBE_RADIUS/7,  .p = {-CFG_GLOBE_RADIUS * 1.6f, +CFG_GLOBE_RADIUS * 1.2f}, .v = {-CFG_GLOBE_RADIUS*1.8, -CFG_GLOBE_RADIUS*1.5},
		           .color = 0xa0f000});

#ifdef HUD_ENABLED
		_setup_huds();
#endif	
	}

#ifdef HUD_ENABLED	
	void _setup_huds()
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

// Housekeeping
	Engine_SFML(sf::RenderWindow& _window)
	      : window(_window)
#ifdef HUD_ENABLED	
			, hud(_window)
#endif
	{
		_setup();
	}
	
};

#endif // __ENGINE_SFML__
