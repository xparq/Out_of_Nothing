// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"
#include "extern/Tracy/public/TracyClient.cpp"

#include "Engine.hpp"
#include "OON_sfml.hpp"

#include <iostream> // For normal user-facing output
	using std::cout, std::endl;
#include <stdexcept>
	using std::exception, std::runtime_error;


// LAST_COMMIT_HASH is defined here:
#include "commit_hash.inc" // The build proc. must put it somewhere on the INCLUDE path!

/*!! TESTING ABORT:
void nested_abort()
{
	Abort("Explicit abort!"s + " let's see how a string"s + " temporary fares...");
}
!!*/

//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
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

	using namespace OON;
	using namespace Szim::diag;

	Args args(argc, argv); // Just for -h -V etc.

	// Explicit manual log level adjustment... The same thing will also be done
	// in SimApp::init, but that's too late for tracing the App ctor logic! :-/
	auto log_level = log::letter_to_level(args("log-level")[0]); //! `args` dependency...
	if (log_level) { log::LogMan::instance()->set_level(log_level); }

	//!! Sad kludge util #348...:
	if (args["?"] || args["h"] || args["help"]) {
		//!!game.show_cmdline_help();
		OONApp_sfml::show_cmdline_help(args);
		return Main.exit_code;
	} else if (args["V"]) {
		cout //! This is not a logging feature, that's why it writes directly to stdout.
			<< "Version: " << LAST_COMMIT_HASH
//!! The build proc. should just send the VDIR tag in a macro! -> #254
#ifdef DEBUG
			<< "-DEBUG"
#endif
#ifdef SFML_STATIC
			<< "-SFML_STATIC"
#endif
			<< '\n';
		return Main.exit_code;
	}

	Main.exit_code = -1;

	try {
		Szim::Engine engine(argc, argv); // args for engine init
			LOGD << "Size of the engine obj.: sizeof(engine) == " << sizeof(engine);
			LOGD << "Size of the app obj.: sizeof(OONApp_sfml) == " << sizeof(OONApp_sfml);
		Main.exit_code = engine.run<OONApp_sfml>(argc, argv); // args for app init
			//! Remember: Main.exit_code won't be set on exceptions!


		LOGI	<< "Profiling stats:\n"
			<< "------------------------------------------------------\n"
			<< "- Use Tracy (and build with `CFLAGS_=-DTRACY_ENABLE`)!\n"
			<< "------------------------------------------------------\n"
		;
/*!! TESTING ABORT():
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
