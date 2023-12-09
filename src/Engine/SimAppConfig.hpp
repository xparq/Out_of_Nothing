#ifndef _DF8M99506BFN4735R9686OK_
#define _DF8M99506BFN4735R9686OK_

#include "Config.hpp"
#include "Time.hpp"
#include "sz/unilang.hh" // AUTO_CONST

class Args; // Enough to actually #include it in the .cpp

namespace Szim {

struct SimAppConfig : Config
{
/*!!
	struct UI
	{
		??? default_bg;
	}
!!*/
	//--------------------------------------------------------------------
	// CUSTOM PROPS...
	AUTO_CONST DEFAULT_WINDOW_WIDTH  = 1280u; // unsigned in SFML
	AUTO_CONST DEFAULT_WINDOW_HEIGHT = 1024u; // unsigned in SFML
	AUTO_CONST WINDOW_WIDTH    = DEFAULT_WINDOW_WIDTH;
	AUTO_CONST WINDOW_HEIGHT   = DEFAULT_WINDOW_HEIGHT;
	AUTO_CONST VIEWPORT_WIDTH  = DEFAULT_WINDOW_WIDTH;
	AUTO_CONST VIEWPORT_HEIGHT = DEFAULT_WINDOW_HEIGHT;
	AUTO_CONST DEFAULT_ZOOM = 0.0000005f; //!! Depends very much on the physics currently! :-/

	AUTO_CONST DEFAULT_SNAPSHOT_FILE_PATTERN = "snapshot_{}.save";
	AUTO_CONST DEFAULT_FPS_LIMIT = 30;

	AUTO_CONST THRUST_FORCE = 6.0e34f; // N (kg*m/s^2)
		//!!Move the rest of these to the Model, too, for now:
		//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
		//!!(They will become props initialized from a real config!)
		
		//! See also: World physics! The specific values here depend on the laws there,
		//! so replacing the physics may very well invalidate these! :-o
		//! The depencendies should be formalized e.g. via using virtual units
		//! provided by the physics there!

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//!! SOME OF THESARE ARE CURRENTLY INITIALIZED BY THE SimApp ctor!
	//!! Possibly move the post-load callback hook here!
	// Sys.
	std::string exe_dir;
	std::string cfg_dir;
	std::string asset_dir;
	std::string engine_state_dir;
	std::string log_dir;
	std::string user_dir;
	std::string session_dir;
	std::string model_dir;
//	std::string addon_dir;
	std::string quick_snapshot_filename_pattern; // Relative paths will be prefixed with session_dir
	// UI
	bool        start_fullscreen;
	std::string window_title;
	std::string default_font_file;
	std::string default_bg_hexcolor; //!! Should be a less arcane "type" (sfw hex color string) + name!
	std::string hud_font_file;
	unsigned    hud_line_height;
	unsigned    hud_line_spacing;
	// Sim.
	Time::CycleCount iteration_limit;
	bool  exit_on_finish; // If iteration_limit > 0, close the app when finished.
	bool  fixed_model_dt_enabled;
	float fixed_model_dt;
	unsigned fps_limit; // 0: no limit
	// UI, presentation
	std::string background_music; //!!?? Awkward... App stuff that needs convenient engine support. How exactly?
	// Misc.
	bool DEBUG_show_keycode = false;

	//----------------------------------------------------------------------------
	SimAppConfig(const std::string& cfg_path, const Args& args, std::string defaults = "");

	SimAppConfig(const SimAppConfig&) = delete; // Could actually be copied _now_, but I'll forget, and make mistakes...
};

} // namespace Szim
#endif // _DF8M99506BFN4735R9686OK_
