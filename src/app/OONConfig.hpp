#ifndef _8PA37GTB7NX73945Y6B2V6C7X245Y45_
#define _8PA37GTB7NX73945Y6B2V6C7X245Y45_

#include "Engine/Config.hpp"
//#include "extern/Args.hpp"
class Args; // Enough to #include it in the .cpp

#include "sz/lang/.hh" // AUTO_CONST

#include "Model/World.hpp"

// Fw.-declare the System config (the app cfg. will have a reference to it):
namespace Szim { class SimAppConfig; }

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

	float player_thrust_force; // N (kg*m/s^2)
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

	// UX
	bool        start_fullscreen;
	std::string default_font_file;
	std::string default_bg_hexcolor; //!! Should be a less arcane "type" (myco hex color string) + name!
	std::string hud_font_file;
	unsigned    hud_line_height;
	unsigned    hud_line_spacing;

	std::string background_music; //!!?? Awkward... App stuff that needs convenient engine support. How exactly?

	// Sim.
	Model::World::GravityMode gravity_mode;
	unsigned chemtrail_burst_particles;
	unsigned exhaust_burst_particles;
	float    exhaust_v_factor;
	float    exhaust_offset_factor; // depends on player v
	float    exhaust_lifetime; // s
	//!!float shield_feed_rate;
	float    shield_depletion_time; // s
	float    shield_recharge_time;  // s
	unsigned shield_burst_particles; // new particles/frame (normalized to 30FPS)

	//----------------------------------------------------------------------------
	OONConfig(const Szim::SimAppConfig& syscfg, const Args& args);
	OONConfig(const OONConfig&) = delete; // Could actually be copied _now_, but I'll forget, and make mistakes...
};

#endif // _8PA37GTB7NX73945Y6B2V6C7X245Y45_
