#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>       //! -> https://www.sfml-dev.org/tutorials/2.5/window-opengl.php
//using namespace sf;

#include <cmath>
#include <memory> // shared_ptr
#include <vector>
#include <iostream> // cerr
using namespace std;


//----------------------------------------------------------------------------
class Render_SFML // "View"
{
public:
	static const auto VIEW_WIDTH  = 800;
	static const auto VIEW_HEIGHT = 600;

	static const auto ALPHA_ACTIVE = 255;
	static const auto ALPHA_INACTIVE = 127;

// Input params
	uint8_t p_alpha;

// Ops
	auto recalc_for_next_frame() // ++world
	// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
	{
		return *this;
	}

// Housekeeping
	Render_SFML()
	      :	p_alpha(ALPHA_ACTIVE)
	{
	}
};


//----------------------------------------------------------------------------
class World_SFML // "Model"
{
struct Physics
{
//	...
};

struct Body
//!!Requires the physics (e.g. constants) of the world!...
{
	float r{0};
	sf::Vector2f p{0, 0};
	sf::Vector2f v{0, 0};

	float density{World_SFML::DENSITY_ROCK / 2};
	float color{0};

	// computed:
	float mass{0};

	//!!not yet possible with designater init:
	//!!Body() : mass(powf(r, 3) * density) {}
	//!!So... (see add_body()):
		void precalc() { mass = powf(r, 3) * density; }
};

// Physics constants -- !!MOVE TO Physics! --:
	//! `const` can't do non-integral statics! :-/
	static constexpr float GLOBE_RADIUS = 50000000; // m
	static constexpr float G = 6.673e-11; //!! No point keeping this real and all the others stretched, 
	               //!! const unless a real orbital simulation is the goal (which isn't)!...
	static constexpr float DENSITY_ROCK = 2000; // kg/m3
	static constexpr float FRICTION = 0.3;

	float V_NUDGE = 12000000; // m/s
	float _SCALE = 0.000001;

// Internal state:
	float dt; // inter-frame increment of the world model time
	sf::Vector2f v = {0, 0};

public: // Just give access for now...:
	vector< shared_ptr<Body> > bodies;

	//!! Move to the renderer!
	vector< shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	vector< shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

public:
// Input params

// Ops
	auto add_body(Body&& obj)
	{
		obj.precalc();
		bodies.push_back(make_shared<Body>(obj));

		// For rendering...

		//! Not all the drawables are also transformables! (E.g. vertex arrays.)
		// (But our little ugly circles are, for now.)
		auto shape = make_shared<sf::CircleShape>(obj.r * _SCALE);
		shapes_to_draw.push_back(shape);
		shapes_to_change.push_back(shape);
	}

	auto recalc_for_next_frame(const Render_SFML& visuals) // ++world
	// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
	{
		dt = clock.getElapsedTime().asSeconds();
		clock.restart();

		for (size_t i = 0; i < bodies.size(); ++i)
		{
			auto& body = bodies[i];

			// Gravity - only apply to the moon(s), ignore the moon's effect on the globe!
			if (i >= 1) {
				auto& globe = bodies[0];
				float distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));
				if (distance < globe->r) distance = globe->r; //!... avoid 0 -> infinity
				float g = G * globe->mass / pow(distance, 2);
				sf::Vector2f gvect((globe->p.x - body->p.x) * g, (globe->p.y - body->p.y) * g);
				sf::Vector2f dv = gvect * (dt);
				body->v += dv;
				sf::Vector2f ds(body->v.x * dt, body->v.y * dt);
				body->p += ds;

cerr << " - gravity: dist = "<<distance << ", g = "<<g << ", gv = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << endl;
			}

			// Friction:
			sf::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
			sf::Vector2f dv = friction_decel * (dt);
			body->v += dv;
			sf::Vector2f ds(body->v.x * dt, body->v.y * dt);
			body->p += ds;
cerr << "v = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
//!!		}

		//! Only generic functions here -- shape[x] is abstract!
//!!		for (auto& shape : shapes_to_change)
//!!		{
			auto shape = dynamic_pointer_cast<sf::CircleShape>(shapes_to_change[i]);

			shape->setFillColor(sf::Color(70 + body->color, 12, 50 - body->color, visuals.p_alpha));

			auto& tshape = dynamic_cast<sf::Transformable&>(*shape);
			tshape.setPosition(sf::Vector2f(
				Render_SFML::VIEW_WIDTH/2  + (body->p.x - body->r) * _SCALE,
				Render_SFML::VIEW_HEIGHT/2 + (body->p.y - body->r) * _SCALE));
		}

		return *this;
	}

	auto move_up()    { bodies[0]->v.y -= V_NUDGE; }
	auto move_down()  { bodies[0]->v.y += V_NUDGE; }
	auto move_left()  { bodies[0]->v.x -= V_NUDGE; }
	auto move_right() { bodies[0]->v.x += V_NUDGE; }

	auto setup()
	{
		//!! Well, we're gonna know these objects by name (index) for now, see recalc():
		// globe:
		add_body({ .r = GLOBE_RADIUS,    .p = {0,0}, .v = {0,0}, .density = DENSITY_ROCK, .color = 10});
		// moons:
		add_body({ .r = GLOBE_RADIUS/10, .p = {-GLOBE_RADIUS * 1.2f, -GLOBE_RADIUS * 1.2f}, .v = {0, 0}, .color = 100});
		add_body({ .r = GLOBE_RADIUS/7,  .p = {-GLOBE_RADIUS * 1.6f, +GLOBE_RADIUS * 1.2f}, .v = {150000, 200000}, .color = 160});
	}

// Housekeeping
	World_SFML()
	{
		setup();
	}

	sf::Clock clock;
};


//----------------------------------------------------------------------------
class Engine_SFML // "Controller"
{
public: // Just give access for now...:
	World_SFML world;
	Render_SFML visuals;

	sf::RenderWindow& window;
public:

// Ops
	auto recalc_for_next_frame()
	// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
	{
		world.recalc_for_next_frame(
			visuals.recalc_for_next_frame()
		);
		return *this;
	}

	auto draw()
	{
	        window.clear();
		for (const auto& entity : world.shapes_to_draw) {
		        window.draw(*entity);
		}
	        window.display();
		return *this;
	}

// Housekeeping
	Engine_SFML(sf::RenderWindow& _window)
	      : window(_window)
	{
	}
};


//============================================================================
int main()
{
	sf::RenderWindow window(sf::VideoMode({Render_SFML::VIEW_WIDTH, Render_SFML::VIEW_HEIGHT}),
		"SFML (OpenGL) Test Drive"); //!, sf::Style::Fullscreen);
//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
//!!??	glEnable(GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?!
//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0

	Engine_SFML engine(window);

	while (window.isOpen()) {
		for (sf::Event event; window.pollEvent(event);) {
			switch (event.type)
			{
			case sf::Event::KeyPressed:
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					window.close(); break;
				case sf::Keyboard::Up:
					engine.world.move_up(); break;
				case sf::Keyboard::Down:
					engine.world.move_down(); break;
				case sf::Keyboard::Left:
					engine.world.move_left(); break;
				case sf::Keyboard::Right:
					engine.world.move_right(); break;
				}
				break;

			case sf::Event::MouseWheelScrolled:
	//				cerr << event.mouseWheelScroll.delta << endl;
				engine.visuals.p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4; // seems to always be 1 or -1
				break;

			case sf::Event::TextEntered:
				if (event.text.unicode > 128) break; // non-ASCII!
				switch (static_cast<char>(event.text.unicode)) {
//				case '+': engine.zoom_in(); break;
//				case '-': engine.zoom_out(); break;
				}
				break;

			case sf::Event::LostFocus:
				engine.visuals.p_alpha = Render_SFML::ALPHA_INACTIVE;
				break;

			case sf::Event::GainedFocus:
				engine.visuals.p_alpha = Render_SFML::ALPHA_ACTIVE;
				break;

			case sf::Event::Closed:
				window.close();
				break;
			}
		}

		engine.recalc_for_next_frame();
		engine.draw();
	}

	return 0;
}
