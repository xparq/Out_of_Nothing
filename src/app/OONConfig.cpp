#include "OONConfig.hpp"

#include "Szim/SimAppConfig.hpp"

#include "extern/Args.hpp" //!! See also in SimApp.hpp!
#include "sz/sys/fs.hh"
	using sz::fs::dirname, sz::fs::endslash_fixup, sz::fs::prefix_by_intent;
#include "sz/str.hh"
//	using sz::to_bool
#include <string>
#include <string_view>

#include "Szim/diag/Error.hpp"
#include "Szim/diag/Log.hpp"


//!! This is an outlier for now: its own config should be reconciled with this,
//!! which in turn should be split from the Engine cfg... -> #272!
//!!#include "Szim/UI/HUD.hpp" // HUD::DEFAULT_LINE_HEIGHT, HUD::DEFAULT_LINE_SPACING
//!! SEE ALSO IN Szim/App/Config.cpp!... :-o


namespace OON {

using namespace Szim;
using namespace std;

//----------------------------------------------------------------------------
OONConfig::OONConfig(const Szim::SimAppConfig& syscfg, [[maybe_unused]] const Args& args) :
	Config(sz::fs::prefix_by_intent(syscfg.base_path(), "OON.cfg"), &syscfg), // Also chain to syscfg!
	syscfg(syscfg) //!! Config has just chained to it, but its '_base' ptr is private! :-/
{
	// 1. Preset hardcoded baseline defaults...
	// ...Well, just default them in one step with loading; see below!
	quick_snapshot_filename_pattern = DEFAULT_SNAPSHOT_FILE_PATTERN;
	gravity_mode = Model::GravityMode::Default;

	// 2. Override from the config...

	//!! These assignments are kinda silly... I don't like storing them here _again_,
	//!! the direct cfg.get() queries could be used, too! Or cache them in the app inst.
	//!! if still needed (for tight loops) -- for another kind of silly duplication. ;)
	//!! Well, the reason we're here is to fixup the inputs, so no raw get() queries
	//!! later!... But: the parsed TOML tables can be modified, too!...
	//!!
	//!!BTW: WITH get() THERE'S NO WAY TO GET VALUES WITHOUT ALWAYS SUPPLYING THE DEFAULTS, TOO! :-/

	// "" is the current dir (e.g. sz::fs::getcwd())

	// Relative to engine_state_dir (unless absolute paths):

	quick_snapshot_filename_pattern = get("snapshot_file_pattern", quick_snapshot_filename_pattern);

	default_bg_hexcolor = get("appearance/colors/default_bg", "#30107080");
	default_font_file   = get("appearance/default_font_file", "gui/font/default.font");
	hud_font_file       = get("appearance/HUD/font_file", default_font_file);

	//!! SEE ALSO IN Szim/App/Config.cpp!... :-o
	hud_line_height     = get("appearance/HUD/line_height", 17); //!!UI::HUD::DEFAULT_LINE_HEIGHT);
	hud_line_spacing    = get("appearance/HUD/line_spacing", 4); //!!UI::HUD::DEFAULT_LINE_SPACING);

	player_thrust_force       = get("sim/player_thrust_force", 1e35f); // N (kg*m/s^2)

	chemtrail_burst_particles = get("sim/chemtrail_particles_add", 5);

	exhaust_burst_particles   = get("sim/exhaust_particles_add", 5);
	exhaust_v_factor          = get("sim/exhaust_v_factor", -1.0f);
	exhaust_offset_factor     = get("sim/exhaust_offset_factor", 0.1f);
	exhaust_lifetime          = get("sim/exhaust_lifetime", 10.f);

	shield_depletion_time     = get("sim/shield_depletion_time", 2.5f); // Sync to snd_shield manually!

	shield_recharge_time      = get("sim/shield_recharge_time", 4.f);
//!!	shield_feed_rate          = get("sim/shield_replenish_rate", 5.f);
	shield_burst_particles    = get("sim/shield_replenish_rate", 5);


	// 3. Process cmdline args to override again...
//!! See also main.cpp! And if main goes to Szim [turning all this essentially into a framework, not a lib, BTW...],
//!! then it's TBD where to actually take care of the cmdline. -- NOTE: There's also likely gonna be an app
//!! configuration/layout/mode, where the client retains its own main()!

	if (args("g-mode") == "R") {
		gravity_mode = Model::GravityMode::Realistic;
		Note("Gravity mode will be set to: "s + "Realistic");
	} else if (args("g-mode") == "H") {
		gravity_mode = Model::GravityMode::Hyperbolic;
		Note("Gravity mode will be set to: "s + "Hyperbolic");
	} else if (args("g-mode") == "0") {
		gravity_mode = Model::GravityMode::Off;
		Note("Gravity will be turned off.");
	}

	//!! 4. Fixup...

	//!! Decide & consolidate whether to go with normalized abs. paths, or keep them as-is,
	//!! and rely on the CWD (which might need some explicit care)!

	background_music = sz::fs::prefix_by_intent(syscfg.asset_dir, background_music);

	LOG << "Current config: " << (current().empty() ? "built-in defaults(!)" : current());
	LOG << "- cfg. base_path: " << base_path();
//!!cerr << "DBG> cfg_dir: " << appcfg_dir << '\n';
}

} // namespace OON
