//https://www.sfml-dev.org/tutorials/2.5/start-vc.php

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>       //! -> https://www.sfml-dev.org/tutorials/2.5/window-opengl.php
using namespace sf;

#include <vector>
#include <iostream>
using namespace std;


//----------------------------------------------------------------------------
class Visuals
//!! This already has SFML dependencies!
{
public:
	static const auto ALPHA_ACTIVE = 255;
	static const auto ALPHA_INACTIVE = 127;

// Input params
	uint8_t p_alpha;

// Ops
	auto recalc_for_next_frame() // ++world
	{
		return *this;
	}

// Housekeeping
	Visuals()
	      :	p_alpha(ALPHA_ACTIVE)
	{
	}
};

//----------------------------------------------------------------------------
class World
//!! This already has SFML dependencies!
{
public: // Just give access for now...:
	sf::CircleShape shape;

public:
// Input params

// Ops
	auto recalc_for_next_frame(const Visuals& visuals) // ++world
	{
		shape.setFillColor(sf::Color(120, 12, 0, visuals.p_alpha));
		return *this;
	}


// Housekeeping
	World()
	      :	shape(100.f)
	{
	}
};


//----------------------------------------------------------------------------
class SFML_Engine
{
public: // Just give access for now...:
	World world;
	Visuals visuals;

	sf::RenderWindow& window;
public:

// Ops
	auto recalc_for_next_frame()
	{
		world.recalc_for_next_frame(
			visuals.recalc_for_next_frame()
		);
		return *this;
	}

	auto draw()
	{
	        window.clear();
	        window.draw(world.shape);
	        window.display();
		return *this;
	}


// Housekeeping
	SFML_Engine(sf::RenderWindow& _window)
	      : window(_window)
	{
	}
/*
	SFML_Engine(sf::RenderWindow& _window, World& _world, Visuals& _visuals)
	      : window(_window), world(_world), visuals(_visuals) // should probably move instead
	{
	}
*/
};


//============================================================================
int main()
{
	sf::RenderWindow window(sf::VideoMode({200, 200}), "SFML (OpenGL)");
//!!?? For SFML + OpenGL mixed mode:
//!!??	glEnable(GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?! https://www.sfml-dev.org/tutorials/2.5/window-opengl.php

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
				window.close();
				break;

			case sf::Event::MouseWheelScrolled:
				cerr << event.mouseWheelScroll.delta << endl;
				engine.visuals.p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4; // seems to always be 1 or -1
				break;

			case sf::Event::LostFocus:
				engine.visuals.p_alpha = Visuals::ALPHA_INACTIVE;
				break;

			case sf::Event::GainedFocus:
				engine.visuals.p_alpha = Visuals::ALPHA_ACTIVE;
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
