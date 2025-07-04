#include "Engine.hpp"

#include "Engine/SimApp.hpp"


namespace Szim {


//--------------------------------------------------------------------
void Engine::__delete_app_if_implicit()
{
	if (app_implicitly_created_ && app_) {
		delete app_;
		app_ = nullptr;
		app_implicitly_created_ = false;
	}
}

//--------------------------------------------------------------------
void Engine::startup()
{
	LOG << "<<< Engine/API starting up... >>>";

	// Safeguard against multiple calls:
	static auto done = false; if (done) return; else done = true; // The exit code may have already been set!

	//!!
	//!! Stuff used to be in SimApp::init():
	//!!

	// Logging...
	// The logger instance(s) has (have) already been through static init, with defaults.
	// Here we just adjust what we can/need, e.g. from command line args etc.
	using namespace diag;

	// Log level override, if requested (with --log-level=<letter>)
	//! NOTE: WAY TOO LATE here for debugging the App ctor init chain (which has already been done)! :-/
	auto log_level = log::letter_to_level(args("log-level")[0]);
	if (log_level) { log::LogMan::instance()->set_level(log_level); }

	//!! Open the file-backed Session Log:
	//!!log::init(log_level, "Szim-debug.log");


	// Apply the config...

	// Misc. fixup that should've been in the ctors, but C++...

	// Some args aren't yet (!!?? can't/shouldn't be?) handled by SimAppConfig itself...

	// UI...
	if (cfg.headless)
		gui.disable();

	// Audio...
	if (cfg.start_muted)
		backend.audio.enabled(false);

	LOG << "<<< Engine/API initialized. >>>";
}

//--------------------------------------------------------------------
void Engine::shutdown()
{
	// Safeguard against multiple calls:
	static auto done = false; if (done) return; else done = true;

	LOG << "<<< Engine/API shutting down... >>>";

	__delete_app_if_implicit(); //!! Belongs to the narrower scope of the executive subsystem, but that doesn't exist yet...

	LOG << "<<< Engine/API shutdown complete. >>>";
}

//--------------------------------------------------------------------
int Engine::__run()
{
	//!! Init the whole engine first...
	//!! This is obviously not the correct place for it, esp. considering that run() should
	//!! even become repeatable..., but at least it's not shoved into the default app init
	//!! any more. :) The ctor would be too early, and offloading this to the user would be very lame.
	startup();
	//!! Note: its counterpart, shutdown(), is called by the destructor, though.

	if (!app_) {
		Bug("Nice try running nullptr as 'App'!...");
		return SadFace;
	}

	// Implicit app pre-init for the app instance (what's left of SimApp::init()):
	app_->internal_app_init();
		//!!
		//!! Handle errors! It currently doesn't signal any, but...
		//!!

//!! To support this "decorated" template init() in the apps the two run() signatures must be allowed
//!! to be different
//!!??NEW:	int exit_code = app_->init(std::forward<Args>(app_init_args)...);
//!!TMP:
	int exit_code = SadFace;
	app_->init();
//!!??		if (app_->terminated()) ...?
	exit_code = app_->exit_code(); //!! init() should just return that

	if (!app_->terminated() && exit_code == SmileyFace) { //!!?? Or... exit_code may be "dirty" without wanting to exit in the old SimApp design?
		Note("Application initialized.");
		LOG << "Engine: App initialized, starting main loop...";


//!!TMP:
		exit_code = app_->SimApp::run(); //!!... Take over: move that logic to __run()!

		// Report
		if (exit_code == SmileyFace) {
			Note("Application finished.");
		} else {
			Fatal("Application aborted.");
		}

		// App cleanup — regardless of happy or sad run (but not after a failed init)...
		//! Note: we may not have created the app instance, so can't rely on RAII here!
		app_->done(); //!!?? What about changing the exit code there? Would people (I) want to do that sometimes?
		Note("Application cleaned up.");

		// Cleanup internal app resoures, do "OnExit" tasks etc.
		app_->internal_app_cleanup();

	} else {
		Fatal("Application setup failed.");
	}

	return exit_code;
}


} // namespace Szim
