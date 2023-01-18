#include "cfg.h"
#include "engine-sfml.hpp"

#include <thread>
#include <iostream> // cerr
using namespace std;

#include <SFML/Graphics.hpp>
//! For mixing native OpenGL context with SFML -> https://www.sfml-dev.org/tutorials/2.5/window-opengl.php:
//#include <SFML/OpenGL.hpp>


//============================================================================
int main(/*int argc char* argv[]*/)
//============================================================================
{
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

#ifdef THREADS_ENABLED
	std::thread engine_updates(&Engine_SFML::update_thread_main_loop, &engine);
			// &engine a) for `this`, b) when this wasn't a member fn, the value form vs ref was ambiguous and failed to compile,
			// and c) the thread ctor would copy the params (by default), and that would be really wonky for the entire engine! :)
#endif

	engine.event_loop();

cerr << "TRACE - before threads join\n";

#ifdef THREADS_ENABLED
	engine_updates.join();
#endif	

	if (!window.setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(true) failed right before exit - ignoring.\n";
		return -1;
	}
	return 0;
}


//============================================================================
//============================================================================
/*
constexpr float Engine::CFG_GLOBE_RADIUS = 50000000; // m
constexpr float Engine::CFG_THRUST_FORCE = 6e34; // N (kg*m/s^2)
constexpr float Engine::CFG_DEFAULT_SCALE = 0.000001; //! This one also depends very much on the physics!
constexpr float Engine::CFG_PAN_STEP = 10; // "SFML defaul pixel" :) (Not quite sure yet how it does coordinates...)
*/