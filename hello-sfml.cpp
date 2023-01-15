#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>       //! -> https://www.sfml-dev.org/tutorials/2.5/window-opengl.php
//using namespace sf;

#include <memory> // shared_ptr
#include <vector>
#include <iostream> // cerr
using namespace std;


//----------------------------------------------------------------------------
// "View"
class Visual_Params //!!?? Is this the UI? Or rendering cfg + params? Or both?
//!! This already has SFML dependencies!
{
public:
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
	Visual_Params()
	      :	p_alpha(ALPHA_ACTIVE)
	{
	}
};


//----------------------------------------------------------------------------
// "Model"
class World
//!! This already has SFML dependencies!
{
	sf::Clock clock;
/*
	float friction = 1; 
	sf::Vector2f v = {0, 0};
	float V_SCALE = 0.000001;
*/
public: // Just give access for now...:
	vector< shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	vector< shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

public:
// Input params

	float dt; // inter-frame increment of the world model time

	float dx = 0, dy = 0;

// Ops
	auto recalc_for_next_frame(const Visual_Params& visuals) // ++world
	// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
	{
		dt = clock.getElapsedTime().asSeconds();
		clock.restart();

/*		// Apply some momentum calc.:
		float decel = - v * friction;
		float dv = a * dt;
		float dx = 0, dy = 0;

		v.x * V_SCALE * dt; dy = v.y * V_SCALE * dt;
		dx = v.x * V_SCALE * dt; dy = v.y * V_SCALE * dt;
*/
		// Here we just know this circle was explicitly created at [0]:
		auto circle = dynamic_pointer_cast<sf::CircleShape>(shapes_to_change[0]);
		circle->setFillColor(sf::Color(120, 12, 0, visuals.p_alpha));

		//! Only generic functions here -- shape[x] is abstract!
		for (auto& shape : shapes_to_change) {
			auto& tshape = dynamic_cast<sf::Transformable&>(*shape);
			tshape.setPosition(tshape.getPosition() + sf::Vector2f(dx, dy));
		}

		dx = 0; dy = 0;  
/*
		if (dx < 1) v.x = 0;
		if (dy < 1) v.y = 0;
*/
		return *this;
	}

	auto move_up()    { dy = -10; }
	auto move_down()  { dy =  10; }
	auto move_left()  { dx = -10; }
	auto move_right() { dx =  10; }
/*
	auto move_up()    { v.y = -10; }
	auto move_down()  { v.y =  10; }
	auto move_left()  { v.x = -10; }
	auto move_right() { v.x =  10; }
*/

// Housekeeping
	World()
	{
		//! Well, we're gonna know this circle by name ([0]), see recalc_for_next_frame:
		auto circle = make_shared<sf::CircleShape>(100.f);
		shapes_to_draw.push_back(circle);
		//! Not all the drawables are also transformables! (E.g. vertex arrays.)
		// (But our little fkn' ugly circle is.)
		shapes_to_change.push_back(circle);
	}
};


//----------------------------------------------------------------------------
// "Controller"
class SFML_Engine
{
public: // Just give access for now...:
	World world;
	Visual_Params visuals;

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
	SFML_Engine(sf::RenderWindow& _window)
	      : window(_window)
	{
	}
};


//============================================================================
int main()
{
	sf::RenderWindow window(sf::VideoMode({200, 200}), "SFML (OpenGL)");
//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
//!!??	glEnable(GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?!
//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0

	SFML_Engine engine(window);

	while (window.isOpen())
	{
	        sf::Event event;
        	while (window.pollEvent(event))
	        {
			//https://www.sfml-dev.org/tutorials/2.5/window-events.php
			switch (event.type)
			{
			case sf::Event::KeyPressed:
				switch (event.key.code)
				{
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

			case sf::Event::LostFocus:
				engine.visuals.p_alpha = Visual_Params::ALPHA_INACTIVE;
				break;

			case sf::Event::GainedFocus:
				engine.visuals.p_alpha = Visual_Params::ALPHA_ACTIVE;
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
