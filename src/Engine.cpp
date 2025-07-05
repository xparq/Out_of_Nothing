#include "Engine.hpp"

#include "Engine/SimApp.hpp"

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"


namespace Szim {


Engine::Engine(int argc, char** argv)
// Note: not creating an app instance here, because the engine can have (run) more than 1 app during its lifetime actually.
// Well, at least conceptually.
	: RuntimeContext(argc, argv)
{
	Note("SimEngine initialized.");

	//! Note: no compulsory __create_app here; we're doing lazy 2-stage init!
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

//----------------------------------------------------------------------------
void Engine::shutdown()
{
	// Safeguard against multiple calls:
	static auto done = false; if (done) return; else done = true;

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

//----------------------------------------------------------------------------
// App lifecycle and process execution orchestrator
//
// - The app can request "early" exit, which we query here via `terminated()` — it's a poor man's
//   exception mechanism; so terminated() signals that the app doesn't want to proceed — either for
//   an error or normal early exit.
// - The app is implicitly initialized with a "happy exit code".
// - The runner first attempts a batch run, which is optional: skipped if its CLI-init returns false.
//   It can also request termination though.
// - Then a normal run follows, unless terminated.
// - All init/done pairs should be matched so that (only) *successful* inits are followed by their
//   corresponding cleanup pair.
//   (Note: we may not have created the app instance ourselves, so can't rely on RAII.)
// - app_->exit_code() should tell whether the result (of a run) was a success or a failure (for reporting)
// - All the important cases should have some reporting.
//
//!! For the lack of any better currently, auto-initializing the engine itself is also done here
//!! (calling startup()). (Its counterpart, shutdown(), is called from the Engine dtor though.)
//
int Engine::__run()
{
	//--------------------------------------------------------------------
	//!! Init the whole engine first...
	//!! This is obviously not the correct place for it, esp. considering that run() should
	//!! even become repeatable..., but at least it's not shoved into the default app init
	//!! any more. :) The ctor would be too early, and offloading this to the user would be very lame.
	startup();
	//!! Note: its counterpart, shutdown(), is called by the destructor, though.
	//--------------------------------------------------------------------

	if (!app_) {
		Bug("Nice try running nullptr as 'App'!...");
		return SadFace;
	}

	// Start with untainted optimism:

		//!! Note: other legacy parts of the system might assume a happy init regardless
		//!! of what __run() is doing, so as a random act of courtesy, let's verify that
		//!! it's actually still the case even without us setting it here:
		assert(app_->exit_code() == SmileyFace);

	app_->exit_code(SmileyFace);

	// Implicit pre-init for the app instance:
	if (app_->internal_app_init()) {

		// Do a CLI-driven batch run first...
		//!! NOT the same as headless: CLI can still be interactive!
		if (app_->init_cli()) {

			Note("Application initialized for CLI/batched operation.");
			LOG << "Application initialized, starting CLI-automated batch loop...";

//!!TMP: The loop itself should be in the Engine:
			app_->SimApp::run_cli_batch();

			// Report
			if (app_->exit_code() == SmileyFace) {
				if (!app_->terminated()) Note("App CLI batch run finished.");
				else                     Note("App CLI batch run finished. Skipping normal startup was requested.");
			} else {
				if (!app_->terminated()) Error("App CLI batch run failed.");
				else                     Error("App CLI batch run failed, aborting!");
			}

			// Cleanup
			if (app_->done_cli()) {
				Note("App cleanup after CLI batch run.");
			} else {
				Error("Error(s) during cleanup after CLI batch run!");
			}
		}

		// Normal run... (if not terminated() yet)
		if (!app_->terminated()) {

			if (app_->init()) {

				Note("Application initialized.");
				LOG << "Application initialized, starting main event loop...";

//!!TMP: The loop itself should be in the Engine:
				app_->SimApp::run();

				// Report
				if (app_->exit_code() == SmileyFace) {
					Note("Application finished.");
				} else {
					if (!app_->terminated()) Error("Application finished with non-zero exit code.");
					else               Error("Application aborted.");
				}

				// Cleanup
				if (app_->done()) {
					Note("Application cleaned up.");
				} else {
					Error("Error(s) during application cleanup!");
				}

			} else {
				Error("Application setup failed!");
			}
		}

		// Cleanup internal app resoures
		if (!app_->internal_app_cleanup()) {
			Error("Error(s) during internal application cleanup!");
		}

	} else {
		Error("Internal application process setup failed!");
	}

	// Single-path exit:
	return app_->exit_code();
}


} // namespace Szim
