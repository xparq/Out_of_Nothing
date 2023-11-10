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

	//---------------------------------------------
	bool select(const std::string& cfg_path); //!! Support comfy cfg tagging, at least by filename!

	std::string get(const std::string& name, std::string def = ""); // 'name' can be "section/name"
	int         get(const std::string& name, int def = 0);
	float       get(const std::string& name, float def = 0.f);
	bool        get(const std::string& name, bool def = false);

	std::string current_cfg() const;
	const std::string& cfg_base_path() const { return _cfg_base_path; }

private:
	std::string _current_config; // path (string)
	std::string _cfg_base_path; //!!TBD: append trailing (back)slash? BEWARE:
	                           //!!It's just a quirk of Windows, Unix etc.; IT'S NOT PART OF THE PATH!!
};

} // namespace Szim
#endif // _SLKWERKJUEIUYCUIUIE12346_
