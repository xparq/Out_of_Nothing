#ifndef _SLKWERKJUEIUYCUIUIE12346_
#define _SLKWERKJUEIUYCUIUIE12346_


#include "Time.hpp"
#include <string>

namespace Szim {

struct Config
{
	// Sys.
	std::string asset_dir;
	// Sim.
	CycleCount iteration_limit;
	bool  fixed_dt_enabled;
	float fixed_dt;

	//!!Legacy hardcoded props:
	std::string WINDOW_TITLE = "Out of Nothing";
	constexpr static auto DEFAULT_FPS_LIMIT = 30;
	unsigned fps_limit = DEFAULT_FPS_LIMIT; // 0: no limit
	        //!! Changing the frame rate would change the physics (in non-fixed-dt mode)
		//!! by increasing the resolution/precision)!!!
	        //!! (Things tend to be more interesting, with more "quantum-like" randomness,
	        //!! with larger dt-s (less precision -> overshoots, tunnelling...)!)

	//!!Legacy PLUS client-level (custom) props...:
	constexpr static float THRUST_FORCE = 6e34f; // N (kg*m/s^2)
		//!!Move the rest of these to the Model, too, for now:
		//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
		//!!(They will become props initialized from a real config!)
		
		//! See also: World physics! The specific values here depend on the laws there,
		//! so replacing the physics may very well invalidate these! :-o
		//! The depencendies should be formalized e.g. via using virtual units
		//! provided by the physics there!


	//---------------------------------------------
	bool select(const std::string& cfg_path); //!! Support comfy cfg tagging, at least by filename!

	std::string get(const std::string& name, std::string def = ""); // 'name' can be "section/name"
	int         get(const std::string& name, int def = 0);
	float       get(const std::string& name, float def = 0.f);
	bool        get(const std::string& name, bool def = false);

	std::string current_cfg() const;
	std::string cfg_base_path() const { return _cfg_base_path; }

private:
	std::string _current_config; // path (string)
	std::string _cfg_base_path; //!!TBD: append trailing (back)slash? BEWARE:
	                           //!!It's just a quirk of Windows, Unix etc.; IT'S NOT PART OF THE PATH!!
};

} // namespace Szim
#endif // _SLKWERKJUEIUYCUIUIE12346_
