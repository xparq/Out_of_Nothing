//!!
//!!	RENAME!...
//!!	Not a subclass of SimApp, but the Backend-specific bits of SimApp itself!
//!!

#include "Engine/SimApp.hpp"
#include "_Backend.hpp"

#include <string>
	using std::string, std::to_string;
	using namespace std::string_literals;
#include <string_view>
	using std::string_view;
#include "sz/fs.hh"
	using sz::dirname, sz::endslash_fixup;
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#include <format>
	using std::format;
#include <iostream>
	using std::cerr, std::endl;
//#include <stdexcept>
//	using std::runtime_error;

using namespace Szim;
//============================================================================
//----------------------------------------------------------------------------
// SimApp ctor.
//
// NOTE: Most init must be done in the member init list, because the `backend`
//      member is polymorphic, and our only chance to set it to the proper type
//      is there. And it requires prior init. of the config, too. Which requires
//      `args` to be initialized as well. And then, if we are at it, I just put
//      the GUI init there, too, for good measure...
//      (The ctor still has work left to do, so its body is not empty though.)
// 
Args ___cpp_workaround_args; // Can't just capture 'args' in the member init list below. :-/
SimApp::SimApp(int argc, char** argv)
	: args(argc, argv, {
		// Long options with 1 param. don't need to be defined:
		//{"moons", 1}, // number of moons to start with
		// Short ones do, unfortunately (they're predicates by default, and don't have '=' to override):
		{"C", 1},
	  })
	// Load & fixup the SimApp config...
	, cfg(
	      ( //! COMMA-OP. HACK TO PRESET CFG. DEFAULTS... (THX FOR NOTHING, C++ ;-p )
		Config::Defaults = R"(
		app_name = "Don't put the app name to the config, FFS!"
		[appearance]
		window_title = "OON <Running with hardcoded defaults!>"
		)"s,
		((___cpp_workaround_args = args) //! ANOTHER ASTONISHING HACK... -- COOL, HUH? ;)
		                                ("cfg")).empty()
		? args("C").empty() ? "" // Use the literal Defaults above now, not the old DEFAULT_CFG_FILE path!
			            : args("C")
		: args("cfg")
/*		((___cpp_workaround_args = args) //! ANOTHER ASTONISHING HACK... -- COOL, HUH? ;)
		                                ("cfg")).empty()
		? args("C").empty()
			? DEFAULT_CFG_FILE
			: args("C")
		: args("C").empty()
			? args("cfg")
			: (cerr << "- WARNING: Both -C and --cfg have been specified; ignoring \"-C " << args("C") << "\"!\n",
			  args("C"))
*/
	      ) //! END OF COMMA-OP. HACK
	      , ___cpp_workaround_args
	//!!Can't convert to SimAppConfig using this! (I had to add a custom SimAppConfig ctor):
	//!!  , [/*args*/](SimAppConfig& cfg_ptr){ _cfg_postload_hook(*cfg_ptr, ___cpp_workaround_args); }
	  )
	// Bootstrap the backend...
	, backend(SFML_Backend::use(cfg))
	// Init the GUI...
	, gui( ((SFML_Backend&)backend).SFML_window(), {
		.basePath = cfg.asset_dir.c_str(), // Trailing / provided by the cfg. fixup!
		.textureFile = "gui/texture.png",
		.bgColor = sfw::Color("#302030d0"),
		.fontFile = cfg.default_font_file.c_str(),
		}, false // Don't manage the window
	  )
//!!	, renderer{View/*!!Not really?...*/::Renderer_SFML::create(backend.hci.window())}
{
/*!! See instead the sad "functional" approach in the member init list above now:
	// Check the cmdline for custom-config location...
	string cfgfile = args("cfg");
	if (cfgfile.empty()) cfgfile = args("C"); // OK if still empty, we'll fall back to the default.
	else if (!args("C").empty())
		cerr << "- WARNING: Both -C and --cfg have been specified; ignoring \"-C " << args("C") << "\"!\n";
	if (cfgfile.empty()) cfgfile = DEFAULT_CFG_FILE;

	// Relative paths will be rooted to the dir of 'cfgfile' by default,
	// i.e. unless it's specifically set in the config
	//!!Move to unilang:
	cfg.select(cfgfile);
	//!!auto basename = fs::path(cfgfile).filename().string();
!!*/

	// Apply the config...
	iterations.max(cfg.iteration_limit);

	if (cfg.fixed_dt_enabled) {
		time.dt_last = cfg.fixed_dt; // Otherwise no one might ever init this...
	}
}


//----------------------------------------------------------------------------
bool SimApp::run()
{
	if (!init()) {
		return false;
	}

	//! The event loop will block and sleep.
	//! The update thread is safe to start before the event loop, but we should also draw something
	//! already before the first event, so we have to release the SFML (OpenGL) Window (crucial!),
	//! and unfreeze the update thread (which would wait on the first event by default).
	if (!((SFML_Backend&)backend).SFML_window().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(false) failed, WTF?! Terminating.\n";
		return false;
	}

	ui_event_state = SimApp::UIEventState::IDLE;

#ifndef DISABLE_THREADS
//!!WAS:std::thread game_state_updates(&OON_sfml::update_thread_main_loop, this);
	std::thread game_state_updates(&SimApp::update_thread_main_loop, this);
		//! NOTES:
		//! - When it wasn't a member fn, the value vs ref. form was ambiguous and failed to compile!
		//! - The thread ctor would *copy* its params (by default), which would be kinda wonky for the entire app. ;)
#endif

	event_loop();

#ifndef DISABLE_THREADS
//cerr << "TRACE - before threads join\n";
	game_state_updates.join();
#endif

	return true;
}

