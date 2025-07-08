//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! Legacy file banner moved here from the now deleted Backend/adapter/SFML/_SimApp.cpp...
//!!
//!! NOTE: That placement, i.e. the direct platform-specific dependencies, is *still* a concern 
//!!       here, too, it's just the new setup (#626, #483) has not yet fully addressed it!
//!!       (I've just put an empty _EngineBoot.cpp to where _SimApp.cpp was, for a reminder...)
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!
//!!	RENAME!!-> Not a subclass of SimApp, just the Backend-specific parts of it
//!!	compiled separately, unlike most/all other backend-specific files here!!
//!!	(See also e.g. _Audio.cpp.inc!...)
//!!
//!!	NOTE: Albeit this is a "safe place" for backend-specific details,
//!!	      it's still best to delegate those to the virtual 'backend'
//!!	      object instead, if possible!
//!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


#include "Engine.hpp"

// For the actual subsystems...

// These are in the .hpp now:
//#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?
//#include "Engine/SimAppConfig.hpp"

#include "Engine/Backend.hpp" // E.g. for convenience accessors of backend components
#include "Engine/Backend/adapter/SFML/_Backend.hpp" //!!... :-/ Use proper dispatching!
#include "Engine/SessionManager.hpp"
#include "Engine/UI.hpp"
//!!#include "Engine/UI/HUD.hpp"
//#include "Engine/UI/Input.hpp"

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"


namespace Szim {


Engine::Engine(int argc, char** argv)
	: args(
		argc, argv,
		{
			// Long options with 1 param (only those with 1?) don't need to be defined.
			// Short ones do, unfortunately (they're predicates by default, and don't have the '=' syntax to disambiguate when aren't):
			{"C", 1}, // Synonym for "cfg"
		}
	)
{
	//----------------------------------------------------------------------------------
	// startup();
	//
	// NO, DON'T init the whole engine here! We're doing a 2-stage lazy init. (-> #483)
	// Note: its counterpart, shutdown(), *is* called by the destructor, though, but just as an
	// extra robustness safeguard, as "half RAII" (or RRID: "Resource Release Is Destruction"...)
	//----------------------------------------------------------------------------------

	LOG << "Engine instance created (to be initialized later).";
}

//--------------------------------------------------------------------
Engine::~Engine()
{
	//!! Don't let ecxeptions leave the dtor, that'd be a gamble against a double-fault termination.
	try {
		shutdown();
		//!! See notes at startup() in run()!
		//!! Putting shutdown here despite startup() is not in the ctor, for the same reason
		//!! the create/delete logic is also asymmetrical! (To support deferred/lazy init.)
	} catch(...) {
		Bug("Unhandled exception during Engine Shutdown!");
	}
}

//--------------------------------------------------------------------
void Engine::startup()
{
	//
	// Reinit guard...
	//
	//!!assert (!__engine_initialized_);
	//!! Don't forbid it; explicitly allow reinit attempts, just to
	//!! help the multi-path run/create/use logic, which is already a PITA...
	if (__engine_initialized_) {
		LOGD << "Engine reinit attempted!"; // See the comments below about the preinitialized default logging capability!
		return;
	}

	Note("<<< Szim Engine starting up... >>>");

//!! #641:
#define BootLOG LOG << BootLOGPrefix_ << // The syntax is also intentionally different from the normal LOG, just to feel even more like a separate facility... :)
	const char* BootLOGPrefix_ = "Engine startup: ";

	//
	// NOTE: `args` is (the only thing that's) been initialized by the ctor for us here.
	//

	//
	// Logging first...
	//
	//!! ...albeit, we don't really know where to log, before reading the config!...
	//
	using namespace diag;
	// Log level override, if requested (with --log-level=<letter>)
	//! NOTE: WAY TOO LATE here for debugging the App ctor init chain (which has already been done)! :-/
	auto log_level = log::letter_to_level(args("log-level")[0]);
	log::LogMan::init({
		.filter_level = log_level ? log_level : log::notice,
		.target = "session.log",
		.fopen_mode = "w+"
	});
	// This would still go through, even if the logger had already been initalized and our init() got ignored:
	//if (log_level) { log::LogMan::set_level(log_level); }
	BootLOG "Logging configured."; //!! Yeah, but this is not the boot logger! (#641)
	                               //!! It's the one single combined everything-logger! :-/
	                               //!! Also: check if the logging has indeed been up, not just assert it! :)

	//
	// Load & fixup the engine config...
	//
	BootLOG "Loading system configuration...";
	syscfg = std::make_unique<EngineConfig>(
//!!	cfg = SimAppConfig(
	/*
		args("cfg").empty()
		? args("C").empty() ? DEFAULT_CFG_FILE // `... ? ""` would use .defaults instead
				: args("C")
		: args("cfg")
	*/
	///* For a pedantic warning:
		args("cfg").empty()
		? args("C").empty()
			? DEFAULT_CFG_FILE
			: args("C")
		: args("C").empty()
			? args("cfg")
			: (Warning("Both -C and --cfg have been specified; ignoring \"-C "s + args("C")),
			args("C"))
	//*/
		, args
		, //! Note: this default config here is pretty redundant, basically only useful for debugging,
		//! as the cfg ctor takes care of the defaults anyway...:
		R"(
		app_name = "Don't put the app name in the config, FFS! ;) "
		[appearance]
		window_title = "Szim-Engine <Initialized with hardcoded defaults!>"
		)"
	); // syscfg

	//!!
	//!! "Apply" the config...
	//!!
	//!! Misc. fixup...
	//!!
	//!! Some args aren't yet (!!?? can't/shouldn't be?) handled by SimAppConfig itself...
	//!!

	//
	// Bootstrap the backend...
	//
	BootLOG "Initializing platform services...";
	backend = &SFML_Backend::use(*syscfg); // `use` returns a ref to a (static) singleton

	//!!Old app-spec. remnants, left here as reminders:
	//!!
	//!!	, renderer{View/*!!Not really?...*/::Renderer_SFML::create(main_window())}
	//!!
	//!! Adapt SessionManager to take a RuntimeContext:
	//!!	, session(*this/*!!, args("session")!!*/)

	//
	// Init the GUI...
	//
	//!!
	//!! This is still entangled with user-level config defaults!
	//!! Actually, even _syscfg itself_ is: e.g. asset_dir should only be a fallback if no app-specific one.
	//!!
	BootLOG "Initializing the UI...";
	gui = std::make_unique<sfw::GUI>(
		backend->as<SFML_Backend>().SFML_window(),
		sfw::Theme::Cfg{
			.basePath = syscfg->asset_dir.c_str(), // Trailing / ensured by the cfg. fixup!
			.textureFile = "gui/texture.png",
			.bgColor = sfw::Color(syscfg->default_bg_hexcolor),
			.fontFile = syscfg->default_font_file.c_str(),
		},
		false // Don't manage the window
	);

	//
	// Audio...
	//
	BootLOG "Initializing Audio...";
	if (syscfg->start_muted) //!! Should have app-level control ("too"?)!
		backend->audio.enabled(false);

	//
	// HEADLESS mode: disable the human interfaces (HCI)! (-> #271)
	//
	// Can't just skip the entire GUI/audio etc. init, because the app would need to be
	// rewritten then at a million places, where it accesses those subsystems directly...
	//
	//!! The app should also have better control over this anyway. It does have at least
	//!! `App/Base.cfg.headless` currently, and does things accordingly (like disabling the
	//!! event loop), but the whole abstraction is weak and dangling; it's not reflected
	//!! (communicated) clearly in the code at all.
	//
	if (syscfg->headless) {
		gui->disable();
		backend->audio.enabled(false);
		BootLOG "HEADLESS mode (with no HCI) activated.";
	}


	__engine_initialized_ = true;
	Note("<<< Szim Engine initialized. >>>");

#undef BootLOG
}

//----------------------------------------------------------------------------
void Engine::shutdown()
{
	// Safeguard against multiple calls:
	static auto done = false; if (done) return; else done = true;

	assert(__engine_initialized_);

	LOG << "<<< Engine/API shutting down... >>>";

	//!! Take care of properly terminating any App(s) still running...
	//!!
	//!! (Well, as the only way to run one is the single-threaded, blocking
	//!! run(), which already takes care of closing apps anyway, this is
	//!! not a thing here just yet...)
	//!!
	//!! There's also the dilemma from the app's perspective: if it wants
	//!! to ensure cleanup by calling its own done its dtor, AND we call the same `done` from
	//!! here before that (i.e. from run(), currently), then there's the
	//!! burden on them to safeguard against a double delete. :-/
	//!! We could just "promise" to always cleanup (which would be a soft
	//!! lie anyway, for various hard breaks we can't handle), but that
	//!! would probably distort the app cleanup logic somewhat. But that's
	//!! probably OK, too: the app logic we define here should be in theory
	//!! agnostic to C++ and RAII; and it wouldn't distort anything for an
	//!! app e.g. in C... :)
	//!! So, there's a warning now in SimApp, for the callbacks
	//!!

	__delete_app_if_implicit(); //!! Belongs to the narrower scope of the executive subsystem, but that doesn't exist yet...

	LOG << "<<< Engine/API shutdown complete. >>>";
}


} // namespace Szim
