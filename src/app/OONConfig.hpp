#ifndef _8PA37GTB7NX73945Y6B2V6C7X245Y45_
#define _8PA37GTB7NX73945Y6B2V6C7X245Y45_

#include "Engine/Config.hpp"
#include "extern/Args.hpp"

#include "sz/unilang.hh" // AUTO_CONST

class Args; // Enough to actually #include it in the .cpp

// Fw.-declare the System config (the app cfg. will have a reference to it):
namespace Szim { struct SimAppConfig; }

struct OONConfig : Szim::Config
{
	const Szim::SimAppConfig& syscfg;
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
	unsigned exhaust_burst_particles;
	float    exhaust_v_factor;
	float    exhaust_offset_factor;
	float    exhaust_lifetime; // s
	// UI, presentation
	std::string background_music; //!!?? Awkward... App stuff that needs convenient engine support. How exactly?
	// Misc.
	bool DEBUG_show_keycode = false;

	//----------------------------------------------------------------------------
	OONConfig(Szim::SimAppConfig& syscfg, const Args& args);

	OONConfig(const OONConfig&) = delete; // Could actually be copied _now_, but I'll forget, and make mistakes...
};

#endif // _8PA37GTB7NX73945Y6B2V6C7X245Y45_
