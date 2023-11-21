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
	AUTO_CONST DEFAULT_WINDOW_WIDTH  = 1024u; // unsigned in SFML
	AUTO_CONST DEFAULT_WINDOW_HEIGHT = 768u;  // unsigned in SFML
	AUTO_CONST WINDOW_WIDTH    = DEFAULT_WINDOW_WIDTH;
	AUTO_CONST WINDOW_HEIGHT   = DEFAULT_WINDOW_HEIGHT;
	AUTO_CONST VIEWPORT_WIDTH  = DEFAULT_WINDOW_WIDTH;
	AUTO_CONST VIEWPORT_HEIGHT = DEFAULT_WINDOW_HEIGHT;

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
	//!! THESARE ARE CURRENTLY INITIALIZED BY THE SimApp ctor!
	//!! Possibly move the post-load callback hook here!
	// Sys.
	std::string data_dir;
	std::string asset_dir;
	std::string window_title;
	std::string default_font_file;
	std::string default_bg_hexcolor; //!! Should be a less arcane "type" (sfw hex color string) + name!
	std::string hud_font_file;
	std::string background_music;
	// Sim.
	Time::CycleCount iteration_limit;
	bool  fixed_model_dt_enabled;
	float fixed_model_dt;
	unsigned fps_limit = DEFAULT_FPS_LIMIT; // 0: no limit
	        //!! Changing the frame rate would change the physics (in non-fixed-dt mode)
		//!! by increasing the resolution/precision)!!!
	        //!! (Things tend to be more interesting, with more "quantum-like" randomness,
	        //!! with larger dt-s (less precision -> overshoots, tunnelling...)!)

	// Misc.
	bool DEBUG_show_keycode = false;

	//----------------------------------------------------------------------------
	SimAppConfig(const std::string& cfg_path, const Args& args);

	SimAppConfig(const SimAppConfig&) = delete; // Could actually be copied _now_, but I'll forget, and make mistakes...
};

} // namespace Szim
#endif // _DF8M99506BFN4735R9686OK_
