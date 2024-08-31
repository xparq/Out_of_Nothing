//!!
//!!	RENAME!!-> Not a subclass of SimApp, just the Backend-specific parts of it
//!!	compiled separately, unlike most/all other backend-specific files here!!
//!!	(See also e.g. _Audio.cpp.inc!...)
//!!
//!!	NOTE: Albeit this is a "safe place" for backend-specific details,
//!!	      it's still best to delegate those to the virtual 'backend'
//!!	      object instead, if possible!
//!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "Engine/SimApp.hpp"
#include "_Backend.hpp"

#include <string>
	using std::string, std::to_string;
	using namespace std::string_literals;
#include <string_view>
	using std::string_view;
#include "sz/fs.hh"
	using sz::dirname;
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
//      is there! And it requires prior init. of the config, too. Which requires
//      `args` to be initialized as well. And then, if we are at it, I just put
//      the GUI init there, too, for good measure...
//      (The ctor still has work left to do, so its body is not empty though.)
// 
SimApp::SimApp(int argc, char** argv, View::ScreenView& main_view)
	: args(argc, argv, {
		// Long options with 1 param. don't need to be defined:
		//{"moons", 1}, // number of moons to start with
		// Short ones do, unfortunately (they're predicates by default, and don't have '=' to override):
		{"C", 1}, {"cfg", 1},
	  })
	// Load & fixup the SimApp config...
	, cfg(args("cfg").empty()
		? args("C").empty() ? DEFAULT_CFG_FILE // `... ? ""` would use .defaults instead
			            : args("C")
		: args("cfg")
/* For a pedantic warning:
		args("cfg").empty()
		? args("C").empty()
			? DEFAULT_CFG_FILE
			: args("C")
		: args("C").empty()
			? args("cfg")
			: (cerr << "- WARNING: Both -C and --cfg have been specified; ignoring \"-C " << args("C") << "\"!\n",
			  args("C"))
*/
	      , args
	      , //! Note: this default config here is pretty redundant, basically only useful for debugging,
		//! as the cfg ctor takes care of the defaults anyway...:
	        R"(
		app_name = "Don't put the app name to the config, FFS! ;) "
		[appearance]
		window_title = "OON <Running with hardcoded defaults!>"
		)"
	  ) // cfg()
	// Bootstrap the backend...
	, backend(SFML_Backend::use(cfg))
	// Init the GUI...
	, gui(((SFML_Backend&)backend).SFML_window(),
	      {
	        .basePath = cfg.asset_dir.c_str(), // Trailing / ensured by the cfg. fixup!
	        .textureFile = "gui/texture.png",
	        .bgColor = sfw::Color(cfg.default_bg_hexcolor),
	        .fontFile = cfg.default_font_file.c_str(),
	      },
	      false // Don't manage the window
	  )
	, _main_view(main_view)
//!!	, renderer{View/*!!Not really?...*/::Renderer_SFML::create(main_window())}
	, session(*this/*!!, args("session")!!*/)
{
/*!! See instead the sad "functional" approach above in the member init list:
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

	this->SimApp::init(); // Our own internal init() is called "secretly", even if overridden...
	                      // (Note: the qualifier is only for emphasis; ctors don't dispatch virtuals.)
}

//----------------------------------------------------------------------------
SimApp::~SimApp()
{
  try { // Let's survive our last moments... :) (Albeit our internal done() is just about empty now...)
	this->SimApp::done(); // Our own internal done() is called "secretly", even if overridden...
	                      // (Note: the qualifier is only for emphasis; dtors don't dispatch virtuals.)
  } catch (...) {
	cerr << "- INTERNAL ERROR: *REALLY UNEXPECTED* exception from SimApp::done()! :-o " << endl;
	//... throw; // <- Could be useful to see the fireworks in DEBUG mode,
	             //    but can't compile without noexcept-violation warnings.
  }
}
