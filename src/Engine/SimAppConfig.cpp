#include "SimAppConfig.hpp"
#include "extern/Args.hpp" //!! See also in SimApp.hpp!
#include "sz/sys/fs.hh"
	using sz::dirname, sz::endslash_fixup, sz::prefix_if_rel;
#include "sz/str.hh"
//	using sz::to_bool

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"

#include <string>
#include <string_view>


//!! This is an outlier for now: its own config should be reconciled with this,
//!! which in turn should be split from the Engine cfg... -> #272!
#include "Engine/UI/hud.hpp" // HUD::DEFAULT_LINE_HEIGHT, HUD::DEFAULT_LINE_SPACING

//!!#undef NDEBUG
#include "sz/DBG.hh"


using namespace Szim;
using namespace std;


//----------------------------------------------------------------------------
SimAppConfig::SimAppConfig(const std::string& cfg_path, const Args& args, std::string defaults)
	: Config(cfg_path, nullptr, defaults)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!! NOTE:
	//!!
	//!! NOT ALL OPTIONS CAN BE SET BOTH IN THE CFG. AND ON THE CMD-LINE!
	//!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// 1. Preset hardcoded baseline defaults...
	// ...Well, just default them in one step with loading; see below!
	window_title = "Out of Nothing"; //!! USE A BUILT-IN APP_NAME RESOURCE/PROP
	quick_snapshot_filename_pattern = DEFAULT_SNAPSHOT_FILE_PATTERN;
	save_compressed = true;

	exe_dir = sz::dirname(args.argv[0]);  //!! See #368, and right below:
	cfg_dir = base_path(); //!! But...: #368 - infer from the exe dir, in Config already!...

	headless = false;
	start_muted = false;

	// 2. Override from the config...

	//!! These assignments are kinda silly... I don't like storing them here _again_,
	//!! the direct cfg.get() queries could be used, too! Or cache them in the app inst.
	//!! if still needed (for tight loops) -- for another kind of silly duplication. ;)
	//!! Well, the reason we're here is to fixup the inputs, so no raw get() queries
	//!! later!... But: the parsed TOML tables can be modified, too!...
	//!!
	//!!BTW: WITH get() THERE'S NO WAY TO GET VALUES WITHOUT ALWAYS SUPPLYING THE DEFAULTS, TOO! :-/
	window_title      = get("appearance/window_title", window_title); //!! not really a cfg option...

	// "" is the current dir (e.g. sz::getcwd())

	asset_dir       = get("asset_dir", "asset/");
	engine_state_dir = get("engine_state_dir", "state");
	// Relative to engine_state_dir (unless absolute paths):
	log_dir         = get("log_dir", "");

	user_dir        = get("user_dir", "user");
	// Relative to user_dir (unless absolute paths):
	session_dir     = get("session_dir", "session");
	model_dir       = get("model_dir", "model");

	quick_snapshot_filename_pattern = get("snapshot_file_pattern", quick_snapshot_filename_pattern);
	save_compressed = get("save_compressed", true);

	start_fullscreen  = get("appearance/start_fullscreen", false);
	default_bg_hexcolor = get("appearance/colors/default_bg", "#30107080");
	default_font_file = get("appearance/default_font_file", "gui/font/default.font");
	hud_font_file     = get("appearance/HUD/font_file", default_font_file);
	hud_line_height   = get("appearance/HUD/line_height", UI::HUD::DEFAULT_LINE_HEIGHT);
	hud_line_spacing  = get("appearance/HUD/line_spacing", UI::HUD::DEFAULT_LINE_SPACING);

	background_music  = get("audio/background_music", "sound/music/background.ogg");

	iteration_limit  = get("sim/loop_cap", -1);
	exit_on_finish   = get("sim/exit_on_finish", false);
	fixed_model_dt   = get("sim/timing/fixed_dt", 0.0333f);
	fixed_model_dt_enabled = get("sim/timing/fixed_dt_enabled", false);
	fps_limit        = get("sim/timing/fps_limit", DEFAULT_FPS_LIMIT);

	global_interactions = get("sim/global_interactions", true);

	player_idle_threshold = DEFAULT_PLAYER_IDLE_THRESHOLD; //!! Make it adjustable!

	DEBUG_show_keycode = get("debug/show_key_codes", false);

	// 3. Process cmdline args to override again...

//!! See also main.cpp! And if main goes to Szim [turning all this essentially into a framework, not a lib, BTW...],
//!! then it's TBD where to actually take care of the cmdline. -- NOTE: There's also likely gonna be an app
//!! configuration/layout/mode, where the client retains its own main()!
	  if (args["fullscreen"]) {
		start_fullscreen = sz::to_bool(args("fullscreen"), sz::str::empty_is_true);
	} if (args["headless"]) {
		headless = true;
	} if (args["snd"]) { //!! Rename to sg. less ambiguous, like "mute" (note: this can be turned back on),
	                     //!! and then use empty_is_false below!
	                     //!! Also add sg. like "silent" or "disable-sound", that wouldn't allow turning it on,
	                     //!! and would also disable the control UI entirely!
		start_muted = !sz::to_bool(args("snd"), sz::str::empty_is_true);
	} if (args["no-save-compressed"]) {
		save_compressed = false;
	} if (args["loop-cap"]) { // Use =0 for no limit (just --loop-cap[=] is ignored!
		try { iteration_limit = stoul(args("loop-cap")); } catch(...) { // stoul crashes on empty! :-/
			Warning("--loop-cap ignored! \"" + args("loop-cap") + "\" must be a valid positive integer."); }
	} if (args["loop_cap"]) { //!! Sigh, the dup...
		try { iteration_limit = stoul(args("loop_cap")); } catch(...) { // stoul crashes on empty! :-/
			Warning("--loop_cap ignored! \"" + args("loop_cap") + "\" must be a valid positive integer."); }
	} if (args["fixed-dt"]) { //!! No "fixed_dt" yet!... :-/
		try {
			if (args("fixed-dt").empty() ) { // stof crashes on empty! :-/
				// Just --fixed-dt should enable it with the configured/default value!
				fixed_model_dt_enabled = true;
			} else {
				fixed_model_dt = stof(args("fixed-dt"));
				fixed_model_dt_enabled = true;
			}
		} catch(...) {
			Warning("--fixed-dt ignored! \"" + args("fixed-dt") + "\" must be a valid floating-pont number.");
		}
	} if (args["fps-limit"]) { // Use =0 for no limit (just --fps-limit[=] is ignored!); but -> #521!

		try { fps_limit = stoul(args("fps-limit")); } catch(...) {
			Warning("--fps-limit ignored! \"" + args("fps-limit") + "\" must be a valid positive integer."); }
	} if (args["fps_limit"]) { //!! Sigh, the dup...
		try { fps_limit = stoul(args("fps_limit")); } catch(...) {
			Warning("--fps_limit ignored! \"" + args("fps_limit") + "\" must be a valid positive integer."); }
	} if (args["dbg-keys"]) {
		DEBUG_show_keycode = true;
	} if (args["interact"]) {
		//!! Either stop telling this, or make it automatic, and tell about all of those then:
		Note("Command-line options (like "s + "--interact" + ") override the config. (" + "cfg/sim/global_interactions" + ").");
		global_interactions = sz::to_bool(args("interact"), sz::str::empty_is_true);
	}

	// Warn about deprecated options (!!should have a proper declarative mechanism for this!!):
	auto _warn_deprecated = [](const char* argname, const char* alt = nullptr) {
		string msg = argname + " is DEPRECATED!"s;
		if (alt) msg += "  Use "s + alt + " instead.";
		Warning(msg);
	};
	const char* argname;
	argname = "--session-no-save"; if (args[argname+2]) _warn_deprecated(argname, "--no-session-autosave");
	argname = "--no-session-save"; if (args[argname+2]) _warn_deprecated(argname, "--no-session-autosave");


	//!! 4. Fixup...

	//!! Decide & consolidate whether to go with normalized abs. paths, or keep them as-is,
	//!! and rely on the CWD (which might need some explicit care)!

	//!! The base dirs must end with a slash currently...:
	sz::endslash_fixup(&engine_state_dir);
	sz::endslash_fixup(&asset_dir);
	sz::endslash_fixup(&user_dir);

	log_dir     = sz::prefix_if_rel(engine_state_dir, log_dir);
	session_dir = sz::prefix_if_rel(user_dir, session_dir);
	model_dir   = sz::prefix_if_rel(user_dir, model_dir);

	if (iteration_limit == 0) iteration_limit = (decltype(iteration_limit))-1; // -1 is what's internally used for no limit
	if (args["exit-on-finish"]) exit_on_finish = (args("exit-on-finish") != "off");
	if (args["exit_on_finish"]) exit_on_finish = (args("exit_on_finish") != "off"); //!! Sigh, the dup...
	background_music = sz::prefix_if_rel(asset_dir, background_music);

	window_title += " ("; window_title += args.exename();
#ifdef DEBUG
	window_title += ", DEBUG build";
#endif
	window_title += ")";


LOGD << "current dir: " << sz::getcwd();
LOGD << "SimApp config: ";
LOGD << " - exe dir: "     << exe_dir; //!! Feels off here, in a config... Why did I put this here (instead of SimApp::)?
LOGD << " - cfg. profile: " << (current().empty() ? "built-in defaults(!)" : current());
LOGD << " - cfg.base_path(): " << base_path();
LOGD << " - cfg_dir: "     << cfg_dir;
LOGD << " - asset_dir: "   << asset_dir;
LOGD << " - default_font: "<< default_font_file;
LOGD << " - engine_state_dir: "   << engine_state_dir;
LOGD << " - log_dir: "     << log_dir;
LOGD << " - user_dir: "    << user_dir;
LOGD << " - session_dir: " << session_dir;
LOGD << " - model_dir: "   << model_dir;
LOGD << " - iteration_limit: " << iteration_limit;
LOGD << " - fixed_model_dt: " << fixed_model_dt << (fixed_model_dt_enabled ? ", enabled" : ", disabled!");
//DBG "save_compressed: " << save_compressed; // Can be seen from the UI, too.
}
