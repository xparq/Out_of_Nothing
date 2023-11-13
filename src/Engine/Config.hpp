#ifndef _SLKWERKJUEIUYCUIUIE12346_
#define _SLKWERKJUEIUYCUIUIE12346_

#include "Time.hpp"
#include <string>
	using namespace std::string_literals; // Damn... Just ...::operator""s did't compile, WTF?!
#include <functional>

namespace Szim {

struct Config
{
	//--------------------------------------------------------------------
	static std::string Defaults;
		// Default in-memory config "file"
		//
		// If the ctor. of select("cfg", ...) is called with an empty
		// path, this "file" will be "loaded" and parsed.
/*!! NOT YET:
		// Will be used as a fallback for missing keys, when queried
		// with `get<type>("key")` instead of `get("key", defval)`,
		// where the type of defval decides the return type.
!!*/
		// Client code is free to set this to whatever, whenever.
		// It will only be used by a subsequent select(), in any of
		// the cfg. instances (!!OF WHICH THERE SHOULD BE ONLY ONE NOW!).

	//--------------------------------------------------------------------
	//!! LEGACY AND/OR CLIENT-LEVEL (CUSTOM) LEFTOVER HARDCODED PROPS!...

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
	//!! EVEN WORSE: SOME UNMANAGED, RANDOMLY ADDED MEMBERS, NOT EVEN OWNED
	//!! BY THIS CLASS! :) INITIALIZED WHEREVER, OR NOT AT ALL...
	//!! ELIMINATE!!! Use the post-load callback for fixups/defaults etc., and
	//!! JUST USE THE GENERIC ["KEY"] QUERIES! (And cache values at the usage site,
	//!! if needed in tight loops!)
	// Sys.
	std::string window_title;
	std::string asset_dir;
	// Sim.
	CycleCount iteration_limit;
	bool  fixed_dt_enabled;
	float fixed_dt;


	//--------------------------------------------------------------------
	using CALLBACK = std::function<void(Config*)>;

	//----------------------------------------------------------------------------
	// Load/parse...
	//----------------------------------------------------------------------------
	//!! Support comfy cfg tagging, at least by filename!

	// Calls select(), throws on error:
	Config(const std::string& cfg_path,
		const CALLBACK& post_load = [](auto&&...) {});

	// Selects ::Defaults if cfg_path is empty:
 	bool select(const std::string& cfg_path, bool can_throw = false,
		const CALLBACK& post_load = [](auto&&...) {});

	//----------------------------------------------------------------------------
	// Typed getters...
	//----------------------------------------------------------------------------
	std::string get(const std::string& name, const char* def = ""); // 'name' can also be "section/name"
	std::string get(const std::string& name, const std::string& def) { return get(name, def.c_str()); }
	//!! Jesuschrist (C++ again), get("...", "default") won't select the string version above,
	//!! but rather some of the numbers below, without the const char* variant!... :-o
	int         get(const std::string& name, int def);
	float       get(const std::string& name, float def);
	bool        get(const std::string& name, bool def);
	//! Alas, only one of these can be meaningfully omit the 2nd arg, to not be ambiguous!

	//----------------------------------------------------------------------------
	// Misc. helpers...
	//----------------------------------------------------------------------------
	std::string current() const; // empty() means using ::Defaults
	std::string base_path() const { return _cfg_base_path; }

private:
	std::string _current_config; // path (string)
	std::string _cfg_base_path; //!!TBD: append trailing (back)slash? BEWARE:
	                           //!!It's just a quirk of Windows, Unix etc.; IT'S NOT PART OF THE PATH!!
};

} // namespace Szim
#endif // _SLKWERKJUEIUYCUIUIE12346_
