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
	//! NOTE: We're being (...umm, supposed to be...) called from a
	//!       locked section of the event loop!

//cerr << "DBG> Doing `while !setActive(FALSE)...`\n";
	// This looping did the trick for #190! (#102 is still a thing, but elsewhere!)
	while (!SFML_window().setActive(false)); //!!?? In case it's already inactive, would it return false?!
	                                         //!!?? Could we not get stuck here forever?! :-o
						 //!!?? As we COULD indeed e.g. in update_thread_main_loop() or event_loop()!
	                                         //!!?? There's no getActive() in SFML to play safe! :-/
//cerr << "DBG> - done `while !setActive(FALSE).`\n";

	SFML_window().create(
		fullscreen ? sf::VideoMode::getDesktopMode()
		           : sf::VideoMode({cfg.WINDOW_WIDTH, cfg.WINDOW_HEIGHT}),
		cfg.window_title,
		fullscreen ? sf::Style::Fullscreen : sf::Style::Resize
	);

	set_frame_rate_limit(_last_fps_limit); //! Restore, as SFML has just killed it with window.create()... :-/

	//!! Doesn't help with #104 (no mouse in fullscreen):
	//!!SFML_window().setMouseCursorVisible(true);

//cerr << "DBG> Doing `while !setActive(TRUE)...`\n";
	while (!SFML_window().setActive(true));
//cerr << "DBG> - done `while !setActive(TRUE).`\n";

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
