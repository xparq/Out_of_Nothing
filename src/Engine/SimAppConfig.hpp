#ifndef _DF8M99506BFN4735R9686OK_
#define _DF8M99506BFN4735R9686OK_

#include "Config.hpp"
#include "Args.hpp"

namespace Szim {

struct SimAppConfig : Config
{
	//--------------------------------------------------------------------
	// CUSTOM PROPS...
	constexpr static auto DEFAULT_WINDOW_WIDTH  = 1024; // unsigned in SFML
	constexpr static auto DEFAULT_WINDOW_HEIGHT = 768;  // unsigned in SFML
	const unsigned WINDOW_WIDTH  = DEFAULT_WINDOW_WIDTH; // unsigned also in SFML
	const unsigned WINDOW_HEIGHT = DEFAULT_WINDOW_HEIGHT;  // unsigned also in SFML
	const unsigned VIEWPORT_WIDTH  = DEFAULT_WINDOW_WIDTH;
	const unsigned VIEWPORT_HEIGHT = DEFAULT_WINDOW_HEIGHT;

	constexpr static auto DEFAULT_FPS_LIMIT = 30;
	unsigned fps_limit = DEFAULT_FPS_LIMIT; // 0: no limit
	        //!! Changing the frame rate would change the physics (in non-fixed-dt mode)
		//!! by increasing the resolution/precision)!!!
	        //!! (Things tend to be more interesting, with more "quantum-like" randomness,
	        //!! with larger dt-s (less precision -> overshoots, tunnelling...)!)

	constexpr static float THRUST_FORCE = 6e34f; // N (kg*m/s^2)
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
	std::string hud_font_file;
	// Sim.
	CycleCount iteration_limit;
	bool  fixed_dt_enabled;
	float fixed_dt;

	//----------------------------------------------------------------------------
	SimAppConfig(const std::string& cfg_path, const Args& args);
};

} // namespace Szim
#endif // _DF8M99506BFN4735R9686OK_
