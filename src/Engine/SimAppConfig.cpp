#include "SimAppConfig.hpp"
#include "extern/Args.hpp" //!! See also in SimApp.hpp!
#include "sz/fs.hh"
	using sz::dirname, sz::endslash_fixup;
#include <string>
#include <string_view>
#include <iostream>

using namespace Szim;
using namespace std;

//----------------------------------------------------------------------------
SimAppConfig::SimAppConfig(const std::string& cfg_path, const Args& args) :
	Config(cfg_path)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!! NOTE:
	//!!
	//!! NOT ALL OPTIONS CAN BE SET BOTH IN THE CFG. AND ON THE CMD-LINE!
	//!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// 1. Preset hardcoded baseline defaults...
	// ...Well, just default them in one step with loading; see below!

	// 2. Override from the config...

	//!! These assignments are kinda silly... I don't like storing them here _again_,
	//!! the direct cfg.get() queries could be used, too! Or cache them in the app inst.
	//!! if still needed (for tight loops) -- for another kind of silly duplication. ;)
	//!! Well, the reason we're here is to fixup the inputs, so no raw get() queries
	//!! later!... But: the parsed TOML tables can be modified, too!...
	//!!
	//!!BTW: WITH get() THERE'S NO WAY TO GET VALUES WITHOUT ALWAYS SUPPLYING THE DEFAULTS, TOO! :-/
	data_dir        = get("data_dir", ""); // "" is the same as sz::getcwd()
	asset_dir       = get("asset_dir", "asset/");
	window_title    = get("appearance/window_title", "Out of Nothing"); //!! USE A BUILT-IN APP_NAME RESOURCE/PROP (that's not a cfg option)!
	default_font_file = get("appearance/default_font_file", "font/default.font");
	hud_font_file   = get("appearance/HUD/font_file", default_font_file);
	iteration_limit = get("sim/loopcap", -1);
	fixed_dt        = get("sim/timing/fixed_dt", 0.f);

	DEBUG_show_keycode = get("debug/show_key_codes", false);

	// 3. Process cmdline args to override again...
//!! See also main.cpp, currently! And if main goes into Szim [turning all this essentially into a framework, not a lib, BTW...],
//!! then it's TBD where to actually take care of the cmdline. -- NOTE: There's also likely gonna be an app
//!! configuration/layout/mode, where the client retains its own main()!
	if   (args["loopcap"]) {
		try { iteration_limit = stoul(args("loopcap")); } catch(...) {
			cerr << "- WRNING: --loopcap ignored! "<<args("loopcap")<<" must be a valid positive integer.\n";
		}
	} if (args["fixed_dt"]) {
		try { fixed_dt = stof(args("fixed_dt")); } catch(...) {
			cerr << "- WRNING: --fixed_dt ignored! "<<args("fixed_dt")<<" must be a valid floating-pont number.\n";
		}
	} if (args["dbg-keys"]) {
		DEBUG_show_keycode = true;
	}

	//!! 4. Fixup...
	//!! Decide & consolidate whether to go with normalized abs. paths, or keep them as-is,
	//!! and rely on the CWD (which might need some explicit care)!
	data_dir = sz::endslash_fixup(data_dir);
	asset_dir = sz::endslash_fixup(asset_dir);
	fixed_dt_enabled = fixed_dt != 0.f;
#ifdef DEBUG	
	window_title += " (DEBUG build)";
#endif	

cerr <<	"DBG> current dir: " << sz::getcwd() << '\n';
cerr <<	"DBG> current(): " << current() << '\n';
cerr <<	"DBG> base_path(): " << base_path() << '\n';
cerr <<	"DBG> asset_dir: " << asset_dir << '\n';
cerr <<	"DBG> data_dir: " << data_dir << '\n';
cerr <<	"DBG> iteration_limit: " << iteration_limit << '\n';
cerr <<	"DBG> fixed_dt_enabled: " << fixed_dt_enabled << '\n';
cerr <<	"DBG> fixed_dt: " << fixed_dt << '\n';
}
