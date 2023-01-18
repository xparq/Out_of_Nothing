#include <SFML/Graphics.hpp>
//! For mixing native OpenGL context with SFML -> https://www.sfml-dev.org/tutorials/2.5/window-opengl.php:
//#include <SFML/OpenGL.hpp>

#include "hud_sfml.hpp"

#include <cmath>
#include <memory> // shared_ptr
#include <thread>
#include <atomic>
#include <vector>
#include <iostream> // cerr
using namespace std;


class Engine_SFML;

//----------------------------------------------------------------------------
class Render_SFML // "View"
{
//----------------------------------------------------------------------------
public:
	static const auto VIEW_WIDTH  = 800;
	static const auto VIEW_HEIGHT = 600;

	static const auto ALPHA_ACTIVE = 255;
	static const auto ALPHA_INACTIVE = 127;

// Input params
	uint8_t p_alpha;

// Internals:
//!!...not quite yet -- just allow access:
public:
	vector< shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	//!!misnomer alert below! should be sg. like "body_images" (as they are not just any Transformables!!! -- these are linked to physical bodies!):
	vector< shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

// Ops
	void render_next_frame(const Engine_SFML& game);

// Housekeeping
	Render_SFML()
	      :	p_alpha(ALPHA_ACTIVE)
	{
	}
};


//----------------------------------------------------------------------------
class World // "Model"
//----------------------------------------------------------------------------
{
public:
struct Physics
{
//	...
};
// Physics constants -- !!MOVE INTO Physics! --:
	//! `const` can't do non-integral statics! :-/
	static constexpr float G = 6.673e-11; //!! No point keeping this real and all the others stretched,
	               //!! const unless a real orbital simulation is the goal (which isn't)!...
	static constexpr float DENSITY_ROCK = 2000; // kg/m3
	static constexpr float FRICTION = 0.3;

public:
struct Body
//! Inner class of World, as it depends on the physics (e.g. constants).
{
	float r{0};
	sf::Vector2f p{0, 0};
	sf::Vector2f v{0, 0};

	float density{World::DENSITY_ROCK / 2};
	float color{0};

	// computed:
	float mass{0};

	//!!not yet possible with designater init:
	//!!Body() : mass(powf(r, 3) * density) {}
	//!!So... (see add_body()):
		void precalc() { mass = powf(r, 3) * density; }
};

public:
	enum Event { None, Collision };

public: // Just allow access for now...:
	vector< shared_ptr<Body> > bodies;

// Ops
	auto add_body(Body&& obj)
	{
		obj.precalc();
		bodies.push_back(make_shared<Body>(obj));
	}

	bool is_colliding(const Body* obj1, const Body* obj2)
	// Takes the body shape into account.
	// Note: a real coll. calc. (e.g. bounding box intersect.) may not need to the distance to be calculated.
	{
		//auto distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));

		return false;
	}

	bool is_colliding(const Body* obj1, const Body* obj2, float distance)
	// Only for circles yet!
	{
		return distance <= obj1->r + obj2->r;
	}

	auto collide(Body* obj1, Body* obj2)
	{
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->v = {0, 0}; // or bounce, or stick to the other body and take its v, or any other sort of interaction.
		//!!...body->p -= ds...;
	}

	auto notify(Event event, Body* obj1, Body* obj2, ...)
	{
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->color += 100;
	}
};

class Engine_SFML;
//----------------------------------------------------------------------------
class World_SFML : public World
{
protected:
// Internal state:
	float dt; // inter-frame increment of the world model time
	sf::Vector2f v = {0, 0};

public:

	void recalc_for_next_frame(const Engine_SFML& game); // ++world

// Housekeeping
	sf::Clock clock;
};


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
	struct Thruster {
		float _throttle = 0;
		//!! :) static constexpr float MyNaN = 2e31f; // to avoid the pain of using the std NAN...
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

public:
	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

};

//----------------------------------------------------------------------------
class Engine_SFML : public Engine
{
friend class Render_SFML;

// Internals... -- not quite yet; just allow access for now:
public:
	World_SFML  world;
	Render_SFML renderer;
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
		_resize_objects(factor);
		_pan_adjust_after_zoom();
	}
	auto zoom_out () { auto factor = 0.80; _SCALE *= factor;
		_resize_objects(factor);
		_pan_adjust_after_zoom();
	}

	void _resize_objects(float factor)
	{
		_transform_objects([factor](sf::Transformable& shape) {
				shape.setScale(shape.getScale() * factor);
		});
	}

	void _transform_objects(const auto& op) // c++20 auto lambda ref (but why the `const` required by MSVC?); https://stackoverflow.com/a/67718838/1479945
	// op = [](Transformable& shape);
	{
		//! Only generic functions here -- Transformable is abstract!
		for (auto& shape : renderer.shapes_to_change) {
			auto& trshape = dynamic_cast<sf::Transformable&>(*shape);
			op(trshape);
		}
	}


	auto updates_for_next_frame()
	// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
	{
		world.recalc_for_next_frame(*this);
		renderer.render_next_frame(*this);
	}

	auto draw()
	{
//!!		sf::Context context; //!! doesn't help with [fix-gl-ctx] & [fix-random-no-shapes]

        window.clear();

		for (const auto& entity : renderer.shapes_to_draw) {
	        window.draw(*entity);
		}

		//!!??If done before the other draws, no shapes would appear, only the text! :-o
#ifdef HUD_ENABLED
		hud.draw(window);
#endif

        window.display();

		if (!window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
			cerr << "\n- [draw] sf::setActive(false) failed!\n";
			terminate();
			return;
		}
	}

	//------------------------------------------------------------------------
	void update_thread_main_loop()
	{
//!!		sf::Context context; //!! doesn't help with [fix-gl-ctx] & [fix-random-no-shapes]

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
//!!		sf::Context context; //!! doesn't help with [fix-gl-ctx] & [fix-random-no-shapes]

		while (window.isOpen() && !terminated()) {
				sf::Event event;
				if (!window.waitEvent(event)) {
					cerr << "- Event processing failed. WTF?! Terminating.\n";
					exit(-1);
				}

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
					}
					break;

				case sf::Event::LostFocus:
					renderer.p_alpha = Render_SFML::ALPHA_INACTIVE;
					break;

				case sf::Event::GainedFocus:
					renderer.p_alpha = Render_SFML::ALPHA_ACTIVE;
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

//			sf::sleep(sf::milliseconds(1000));
//			cerr << "- CYCLE TAIL of while (window.isOpen && !terminated)..." <<endl;

/* This seems to be redundant here, but anyway: not doing window.close on Esc solved [fix-setactive-fail]
   at least for this particular case:
			if (!window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
				cerr << "\n- [event_loop] sf::setActive(false) failed!\n";
				terminate();
				return;
			}
*/
			ui_event_state = UIEventState::EVENT_READY;
		}
	}

	auto add_body(World_SFML::Body&& obj)
	{
		world.add_body(std::forward<decltype(obj)>(obj));

		// For rendering...

		//! Not all the Drawables are also Transformables! (E.g. vertex arrays.)
		// (But our little ugly circles are, for now.)
		auto shape = make_shared<sf::CircleShape>(obj.r * _SCALE);
		renderer.shapes_to_draw.push_back(shape);
		renderer.shapes_to_change.push_back(shape); // "to transform"
	}

	auto _setup()
	{
		//!! Well, we're gonna know these objects by name (index) for now, see recalc():
		// globe:
		add_body({ .r = CFG_GLOBE_RADIUS,    .p = {0,0}, .v = {0,0}, .density = world.DENSITY_ROCK, .color = 20});
		// moons:
		add_body({ .r = CFG_GLOBE_RADIUS/10, .p = {CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -CFG_GLOBE_RADIUS * 2}, .color = 100});
		add_body({ .r = CFG_GLOBE_RADIUS/7,  .p = {-CFG_GLOBE_RADIUS * 1.6f, +CFG_GLOBE_RADIUS * 1.2f}, .v = {-CFG_GLOBE_RADIUS*1.8, -CFG_GLOBE_RADIUS*1.5}, .color = 160});

#ifdef HUD_ENABLED
		_setup_huds();
#endif	
	}

#ifdef HUD_ENABLED	
	void _setup_huds()
	{
		hud.add("FPS: ?");
		hud.add("globe mass", &world.bodies[0]->mass);
	//!!	hud.add([this]()->string { return to_string(this->world.clock.getElapsedTime().asSeconds()); });
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

//============================================================================
int main(/*int argc char* argv[]*/)
//============================================================================
{
//!!	sf::Context; // seems 2 b redundant also here; -> the other instances commented out!

	auto window = sf::RenderWindow(
		sf::VideoMode({Render_SFML::VIEW_WIDTH, Render_SFML::VIEW_HEIGHT}),
		"SFML (OpenGL) Test"
	); //!, sf::Style::Fullscreen);
	//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
	//!!??
	//sf::glEnable(sf::GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?!
	//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0

	Engine_SFML engine(window);

	//! The event loop will block and sleep.
	//! The update thread is safe to start before the event loop, but we should also draw something
	//! already before the first event, so we have to release the SFML (OpenGL) Window (crucial!),
	//! and unfreeze the update thread (which would wait on the first event by default).
	if (!engine.window.setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(false) failed, WTF?! Terminating.\n";
		return -1;
	}

	engine.ui_event_state = Engine::UIEventState::IDLE;

	std::thread engine_updates(&Engine_SFML::update_thread_main_loop, &engine);
			// &engine a) for `this`, b) when this wasn't a member fn, the value form vs ref was ambiguous and failed to compile,
			// and c) the thread ctor would copy the params (by default), and that would be really wonky for the entire engine! :)

	engine.event_loop();

cerr << "TRACE - before threads join\n";

	engine_updates.join();

	if (!window.setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(true) failed right before exit - ignoring.\n";
		return -1;
	}
	return 0;
}

//============================================================================
// Memeber fn. implementations...
//============================================================================
void World_SFML::recalc_for_next_frame(const Engine_SFML& game) // ++world
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	dt = clock.getElapsedTime().asSeconds();
	clock.restart();

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Thrust:
		if (i == 0) {
			sf::Vector2f F_thr( (-game.thrust_left.throttle() + game.thrust_right.throttle()) * dt,
							    (-game.thrust_up.throttle() + game.thrust_down.throttle()) *dt);
			body->v += (F_thr / body->mass);
		}

		// Gravity - only apply to the moon(s), ignore the moon's effect on the globe!
		if (i >= 1) {
			auto& globe = bodies[0];
			//float distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));
			float dx = globe->p.x - body->p.x,
			      dy = globe->p.y - body->p.y;
			float distance = sqrt(dx*dx + dy*dy);

			//! Collision det. is crucial also for preventing 0 distance to divide by!
			//!!Collision and "distance = 0" are not the samne things!
			//!!Collision concerns the surfaces, but "distance" here for gravity
			//!!is measured from the center (of mass) of the body!
			//!!But d = 0 is a "subset" of collisions, so if there would be a "safe"
			//!!adjustment for any collision to guarantee distance != 0, that would be
			//!!enough...

			//!!Also, obviously, "almost 0" could be just as bad as 0 (which is not
			//!!even quite real in float, and would anyway just yield a NaN), so
			//!!the real solution would be smarter modeling of the high-speed case
			//!!to do what nature does... Dunno, create a black hole, if you must! :)

			if (/*physics.*/is_colliding(body.get(), globe.get(), distance)) {
			//! Done before actually reaching the body (so we can avoid the occasional frame showing the penetration :) )!
			//  (Opting for the perhaps even less natural "but didn't even touch!" issue...)

				// If the collision results in a well-defined fixed position/velocity etc.,
				// they may need special treatment, because at this point the checked body may not
				// yet have reached (or crossed the boundary of) the other, so it needs to be adjusted
				// to the expected collision end-state!
				/*physics.*/collide(body.get(), globe.get());

				//! Also call a high-level, "predefined emergent" interaction "hook":
				notify(Event::Collision, body.get(), globe.get());
			} else {
 				float g = G * globe->mass / (distance * distance);
				sf::Vector2f gvect(dx * g, dy * g);
				//!!should rather be: sf::Vector2f gvect(dx / distance * g, dy / distance * g);
				sf::Vector2f dv = gvect * dt;
				body->v += dv;
//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<body->v.x<<","<<body->v.y<<") " << endl;
			}
		}

		// Friction:
		sf::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
		sf::Vector2f dv = friction_decel * (dt);
		body->v += dv;
		sf::Vector2f ds(body->v.x * dt, body->v.y * dt);

		body->p += ds;
//cerr << "v["<<i<<"] = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
	}
}

//============================================================================
void Render_SFML::render_next_frame(const Engine_SFML& game)
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// The indexes match the corresponding physical bodies!
	for (size_t i = 0; i < shapes_to_change.size(); ++i)
	{
		auto& body = game.world.bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(shapes_to_change[i]);
		shape->setFillColor(sf::Color(70 + body->color, 12, 50 - body->color, p_alpha));

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

		trshape.setPosition(sf::Vector2f(
			VIEW_WIDTH/2  + (body->p.x - body->r) * game._SCALE + game._OFFSET_X,
			VIEW_HEIGHT/2 + (body->p.y - body->r) * game._SCALE + game._OFFSET_Y));
	}
}
