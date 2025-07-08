// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"
#include "extern/Tracy/public/TracyClient.cpp"

#include "Engine.hpp"
#include "OON_sfml.hpp"

#include <iostream> // For normal user-facing output
	using std::cout, std::endl;
#include <stdexcept>
	using std::exception, std::runtime_error;


/*!! TESTING `ABORT`:
void nested_abort()
{
	Abort("Explicit abort!"s + " let's see how a string"s + " temporary fares...");
}
!!*/

//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
	//!! If we want to log before the engine has been initialized,
	//!! AND we want to have the same logging setup, THEN we must do
	//!! the same init here ourselves, BEFORE the first logging attempt:
	Args args(argc, argv);
	using namespace Szim::diag;
	auto log_level = log::letter_to_level(args("log-level")[0]);
	log::LogMan::init({
		.filter_level = log_level ? log_level : log::notice,
		.target = "session.log",
		.fopen_mode = "w+" // truncate
	});
	// Or...
	// ...we can pre-configure the logger differently, and the engine will just
	// have to live with that... ;)
	//auto log_level = log::letter_to_level('N');
	//if (log_level) { log::LogMan::set_level(log_level); } // This would default-init for console output!
	//log::LogMan::init({.filter_level = log::notice, .target = "default.log"});
	// Or...
	// ...we can wait for the engine init with our first log message, and
	// then optionally tune the logger (but can't fully reconfigure it).

	struct Main_ {
		int exit_code = 0;
		// Only log levels up to "notice" are enabled by default (before main)!
		// But that's overridden to "info" (in Log.hpp, by SZ_LOG_USE_DEFAULT_LEVEL)...
		Main_()  { LOGI << "main() entered..."; }
		~Main_() {
			if      (exit_code < 0) LOGE << "main() returning: " << exit_code;
			else if (exit_code > 0) LOGW << "main() returning: " << exit_code;
			else                    LOGI << "main() returning: " << exit_code;
			//!! Alas, the underlying PLOG* macros conflict with just this:
			//!!(exit_code ? LOGW : LOGI) << "main() returning: " << exit_code;
		}

	} Main;


	//using namespace Szim;

	//
	// Go...
	//
	Main.exit_code = -1;
	try {
/*
		using namespace Szim::diag;
		//!! Explicit manual pre-boot log level adjustment... The same thing will also be done
		//!! in Engine::startup, but we're free to preset our own settings here!
		auto log_level = log::letter_to_level('W');
		if (log_level) { log::LogMan::set_level(log_level); }
*/
		using namespace OON;
		Szim::Engine engine(argc, argv); // args for engine init
			LOGD << "Size of the engine obj.: sizeof(engine) == " << sizeof(engine);
			LOGD << "Size of the app obj.: sizeof(OONApp_sfml) == " << sizeof(OONApp_sfml);
		Main.exit_code = engine.run<OONApp_sfml>(argc, argv); // args for app init
			//! Remember: Main.exit_code won't be set on exceptions.

		// Alternatively, if the (SimApp-derived) app is created outside the engine (e.g. here):
		// MyApp app(engine.context());
		// Main.exit_code = engine.run<OONApp_sfml>(app, argc, argv);

		LOGI	<< "Profiling stats:\n"
			<< "------------------------------------------------------\n"
			<< "- Use Tracy (and build with `CFLAGS_=-DTRACY_ENABLE`)!\n"
			<< "------------------------------------------------------\n"
		;
/*!! TESTING `ABORT`:
if (argv) {
	nested_abort();
	//Bug("Oh, crap!");
	//Abort("Explicit abort!"s + "let's see how a string temporary fares");
	cout << "- ...wait, but it didn't abort actually?! WTF??? :-o ";
}
!!*/
	} catch (const Szim::diag::FatalError& x) { //!! Do sg. about this ugly low-level-internal-detail name!... :-/
	                                            //!! Well, it'll disappear when moving from this direct-main to an app-runner setup!...
		// Message already delivered by Fatal()/Abort()...
		cout << x.what() << endl;
	} catch (const runtime_error& x) {
		Error( ("`std::runtime_error` exception: "s + x.what()).c_str() );
	} catch (const exception& x) {
		Error( ("`std::exception`: "s + x.what()).c_str() );
	} catch (...) {
		Error("UNKNOWN EXCEPTION!");
	}
	return Main.exit_code;
}
