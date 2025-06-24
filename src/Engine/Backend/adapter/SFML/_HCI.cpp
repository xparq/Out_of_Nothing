#include "_HCI.hpp"

#include <string>
	using std::string;

#include "Engine/diag/Log.hpp"

using namespace Szim;

//----------------------------------------------------------------------------
//!!void SFML_HCI::create_main_window(const Config& cfg) //override
//!!void SFML_HCI::Window::create(const Window::Config& requested_cfg) //override
SFML_HCI::Window::Window(const Window::Config& initial_cfg)
	: HCI::Window::Window(initial_cfg) //!! Might need to save the requested config!
{
	//!! THIS IS LAME! HEADLESS support should NOT percolate down to this level,
	//!! instead the HCI subsystem should simply not create a window (or create
	//!! a dummy one)! .headless shouldn't even be a Window.cfg member!
	if (cfg.headless) {
		LOG << "HEADLESS mode: Main window NOT created.";
		return;
	}

//cerr << "Creating the window...\n";
	if (cfg.fullscreen) {
//cerr <<"--- desktop mode valid?! "<< sf::VideoMode::getDesktopMode().isValid() <<'\n';
		auto modes = sf::VideoMode::getFullscreenModes();
//cerr <<"--- fullscreen mode[0]: "<< modes[0].size.x <<", " << modes[0].size.y <<'\n';
//cerr <<"--- fullscreen mode[1]: "<< modes[1].size.x <<", " << modes[1].size.y <<'\n';
		auto& mode = modes[0];
		_owned_sfml_window.create(mode, //!! Should be a control to select!
			cfg.title,
			sf::Style::Default,
			sf::State::Fullscreen);
		//!!
		//!! ERROR CHK!... BUT HOW?
		//!!
		//!!if (ok) { // adjust the size to reality...:
			cfg.width  = mode.size.x;
			cfg.height = mode.size.y;
			//!! We might need to save the requested dimensions!
		//!!}
	} else {
		_owned_sfml_window.create(sf::VideoMode({cfg.width, cfg.height}), cfg.title);
	}
//cerr << "...done.\n";
	//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
	//!!??
	//sf::glEnable(sf::GL_TEXTURE_2D); //!!?? why would this bw needed, if SFML already draws into an OpenGL canvas?!
	//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0

//!!??	[width, height] = _owned_sfml_window.getSize(); // I think C++ still can't do this. :-/
	cfg.width  = _owned_sfml_window.getSize().x;
	cfg.height = _owned_sfml_window.getSize().y;
	//!! We might need to save the requested dimensions!

	LOGI << "HCI Window size set to: "<<cfg.width<<" x "<<cfg.height;
}

//----------------------------------------------------------------------------
SFML_HCI::SFML_HCI(SimAppConfig& syscfg) :
	syscfg(syscfg),
	_main_window({syscfg.WINDOW_WIDTH,
	              syscfg.WINDOW_HEIGHT,
	              syscfg.window_title,
	              syscfg.start_fullscreen,
	              syscfg.headless})
{
	set_frame_rate_limit(syscfg.fps_limit);
}

//----------------------------------------------------------------------------
void SFML_HCI::switch_fullscreen(bool fullscreen) // override
//!
//! NOTE: (Supposedly...) called from a locked section of the event loop!
//!
{
	//!! THIS IS LAME! HEADLESS support should NOT percolate down to this level,
	//!! instead the HCI subsystem should simply not create a window (or create
	//!! a dummy one)! .headless shouldn't even be a Window.cfg member!
	if (syscfg.headless) return;


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
		           : sf::VideoMode({syscfg.WINDOW_WIDTH, syscfg.WINDOW_HEIGHT}),
		syscfg.window_title,
		fullscreen ? sf::Style::Default : sf::Style::Resize,
		fullscreen ? sf::State::Fullscreen : sf::State::Windowed
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
	// Didn't fail IF the process has kept running (!!well, in theory... it still might IRL,
	// as there's no check/exception to abort!!), so let's just blatantly store what came in:
	_main_window.cfg.fullscreen = fullscreen;

	_main_window.cfg.width  = SFML_window().getSize().x;
	_main_window.cfg.height = SFML_window().getSize().y;
	LOGI << "HCI Window size updated to: "<<_main_window.cfg.width<<" x "<<_main_window.cfg.height;
}

//----------------------------------------------------------------------------
void SFML_HCI::set_frame_rate_limit(unsigned fps) //override
{
	_last_fps_limit = fps;
	SFML_window().setFramerateLimit(fps); // 0: no limit
}
