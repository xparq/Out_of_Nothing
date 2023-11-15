#include "SimAppConfig.hpp"
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
	// 1. Preset hardcoded baseline defaults...
	//!!?? Or just set them all in .value_or() (see below)?

	// 2. Override those from the loaded ..

	//!! Thse assignments are so silly... Either don't store them there (again!),
	//!! but just use the queries [and the reason we're here is to fixup
	//!! missing values with defaults... -- !!ALSO, THERE'S NO WAY TO GET
	//!! VALUES WITHOUT ALWAYS SUPPLYING THEIR DEFAULTS, TOO, ALL THE TIME!...],
	//!! or store (cache) them somewhere else (in the app inst.)!
	asset_dir       = get("fs-layout/asset_dir", sz::getcwd() + "/asset/"); //!! Trailing / still required!
	iteration_limit = get("sim/loopcap", -1);
	fixed_dt        = get("sim/timing/fixed_dt", 0.f);

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
	}

	//!! 4. Fixup...
	//!! Decide & consolidate whether to go with normalized abs. paths, or keep them as-is,
	//!! and rely on the CWD (which might need some explicit care)!
	data_dir = sz::endslash_fixup(get("data_dir", ""));
	asset_dir = get("fs-layout/asset_dir", sz::getcwd() + "/asset/"); //!! Trailing / still required!
	
	fixed_dt_enabled = fixed_dt != 0.f;
	window_title = get("appearance/window_title", "Out of Nothing") //!! USE A BUILT-IN APP_NAME RESOURCE/PROP (that's not a cfg option)!
#ifdef DEBUG	
	+ " (DEBUG build)";
#endif	
	;

cerr <<	"DBG> current dir: " << sz::getcwd() << '\n';
cerr <<	"DBG> current(): " << current() << '\n';
cerr <<	"DBG> base_path(): " << base_path() << '\n';
cerr <<	"DBG> asset_dir: " << asset_dir << '\n';
cerr <<	"DBG> data_dir: " << data_dir << '\n';
cerr <<	"DBG> iteration_limit: " << iteration_limit << '\n';
cerr <<	"DBG> fixed_dt_enabled: " << fixed_dt_enabled << '\n';
cerr <<	"DBG> fixed_dt: " << fixed_dt << '\n';
}
