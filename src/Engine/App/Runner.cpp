#include "Engine.hpp"

#include "Engine/App.hpp"

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"


namespace Szim {


//--------------------------------------------------------------------
// See its __create_app_implicitly counterpart in the header (because it's templated)!
//--------------------------------------------------------------------
void Engine::__delete_app_if_implicit()
{
	if (app_implicitly_created_ && app_) {
		delete app_;
		app_ = nullptr;
		app_implicitly_created_ = false;
	}
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
int Engine::__run()
{
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
