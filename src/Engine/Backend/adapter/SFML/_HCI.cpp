#include "_HCI.hpp"

#include <string>
	using std::string;
#include <iostream> // cerr, for debugging only!...
	using std::cerr;

using namespace Szim;

//----------------------------------------------------------------------------
SFML_HCI::Window::Window(unsigned width, unsigned height, const char* title, bool fullscreen)
{
//cerr << "Creating the window...\n";
	if (fullscreen) {
//cerr <<"--- desktop mode valid?! "<< sf::VideoMode::getDesktopMode().isValid() <<'\n';
		auto modes = sf::VideoMode::getFullscreenModes();
//cerr <<"--- fullscreen mode[0]: "<< modes[0].size.x <<", " << modes[0].size.y <<'\n';
//cerr <<"--- fullscreen mode[1]: "<< modes[1].size.x <<", " << modes[1].size.y <<'\n';
		auto& mode = modes[0];
		_owned_sfml_window.create(mode, //!! Should be a control to select!
			title, sf::Style::Fullscreen);
		//!!
		//!! ERROR CHK!... BUT HOW?
		//!!
		//!!if (ok) { // adjust the size to reality...:
			width  = mode.size.x;
			height = mode.size.y;
		//!!}
	} else {
		_owned_sfml_window.create(sf::VideoMode({width, height}),
			title);
	}
//cerr << "...done.\n";
	//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
	//!!??
	//sf::glEnable(sf::GL_TEXTURE_2D); //!!?? why would this bw needed, if SFML already draws into an OpenGL canvas?!
	//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0

//!!??	[this->width, this->height] = _owned_sfml_window.getSize();
	this->width  = _owned_sfml_window.getSize().x;
	this->height = _owned_sfml_window.getSize().y;
//!! Or:
//!!	this->width  = width;
//!!	this->height = height;
cerr << "DBG> "<<__FUNCTION__<<": HCI Window size set to: "<<width<<" x "<<height<<"\n";
}

//----------------------------------------------------------------------------
SFML_HCI::SFML_HCI(SimAppConfig& syscfg) :
	cfg(syscfg),
	_main_window(cfg.WINDOW_WIDTH, cfg.WINDOW_HEIGHT, cfg.window_title.c_str(), syscfg.start_fullscreen)
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

	//!! Move this .create() to Window, and just call from here (as well as from its ctor)!
	SFML_window().create(
		fullscreen ? sf::VideoMode::getDesktopMode()
		           : sf::VideoMode({cfg.WINDOW_WIDTH, cfg.WINDOW_HEIGHT}),
		cfg.window_title,
		fullscreen ? sf::Style::Fullscreen : sf::Style::Resize
	);

	set_frame_rate_limit(_last_fps_limit); //! Restore, as SFML has just killed it with window.create()... :-/
	                                       //! (Also covers the initial case of just 0 (see HCI.hpp!).)

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

	_main_window.width  = SFML_window().getSize().x;
	_main_window.height = SFML_window().getSize().y;
cerr << "DBG> "<<__FUNCTION__<<": HCI Window size updated to: "<<_main_window.width<<" x "<<_main_window.height<<"\n";
}

//----------------------------------------------------------------------------
void SFML_HCI::set_frame_rate_limit(unsigned fps) //override
{
	_last_fps_limit = fps;
	SFML_window().setFramerateLimit(fps); // 0: no limit
}
