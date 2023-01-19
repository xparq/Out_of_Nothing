#include "cfg.h"
#include "engine-sfml.hpp"

#include <thread>
#include <iostream> // cerr
using namespace std;

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
//! For mixing native OpenGL context with SFML -> https://www.sfml-dev.org/tutorials/2.5/window-opengl.php:
//#include <SFML/OpenGL.hpp>

//============================================================================
int main(/*int argc char* argv[]*/)
//============================================================================
{
	auto window = sf::RenderWindow(
		sf::VideoMode({Renderer_SFML::VIEW_WIDTH, Renderer_SFML::VIEW_HEIGHT}),
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
