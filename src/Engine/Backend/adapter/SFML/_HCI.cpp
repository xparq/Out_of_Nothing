#include "_HCI.hpp"

#include <string>
	using std::string;
#include <iostream> // cerr, for debugging only!...
	using std::cerr;

using namespace Szim;

//----------------------------------------------------------------------------
SFML_HCI::Window::Window(unsigned width, unsigned height, const char* title)
{
	_owned_sfml_window.create(sf::VideoMode({width, height}), title);
	//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
	//!!??
	//sf::glEnable(sf::GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?!
	//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0
}

//----------------------------------------------------------------------------
SFML_HCI::SFML_HCI(SimAppConfig& syscfg) :
	cfg(syscfg),
	_main_window(cfg.WINDOW_WIDTH, cfg.WINDOW_HEIGHT, cfg.window_title.c_str())
{
	set_frame_rate_limit(cfg.fps_limit);
}

//----------------------------------------------------------------------------
void SFML_HCI::switch_fullscreen(bool fullscreen) // override
{
	//! NOTE: We're being called here from a locked section of the event loop.

	if (SFML_window().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
cerr << "\n- [toggle_fullscreen] sf::setActive(false) failed!\n";
	}

	SFML_window().create(
		fullscreen ? sf::VideoMode::getDesktopMode()
		           : sf::VideoMode({cfg.WINDOW_WIDTH, cfg.WINDOW_HEIGHT}),
		cfg.window_title,
		fullscreen ? sf::Style::Fullscreen|sf::Style::Resize/*!!??*/ : sf::Style::Resize
	);

	set_frame_rate_limit(_last_fps_limit); //! Restore, as SFML has just killed it with window.create()... :-/

	if (!SFML_window().setActive(true)) { //https://stackoverflow.com/a/23921645/1479945
cerr << "\n- [toggle_fullscreen] sf::setActive(true) failed!\n";
	}

//	if (!(is_full = !is_full) /* :) */) {
//		// full
//	} else {
//		// windowed
//	}
}

//----------------------------------------------------------------------------
void SFML_HCI::set_frame_rate_limit(unsigned fps) //override
{
	_last_fps_limit = fps;
	SFML_window().setFramerateLimit(fps); // 0: no limit
}
