// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"
#include "extern/Tracy/public/TracyClient.cpp"

#include "OON_sfml.hpp"
#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"

#include <iostream> // For normal user-facing output
	using std::cout, std::endl;
#include <stdexcept>
	using std::exception, std::runtime_error;


// LAST_COMMIT_HASH is defined here:
#include "commit_hash.inc" // The build proc. must put it somewhere on the INCLUDE path!


//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
	struct Main_ {
		int exit_code = 0;
		// Only log levels up to "notice" are enabled by default (before main)!
		// But that's overridden to "info" (in Log.hpp, by SZ_LOG_USE_DEFAULT_LEVEL)...
		Main_()  { LOGI << "main() entered..."; }
		~Main_() { LOGI << "main() returning:" << exit_code; }
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
		OONApp_sfml game(argc, argv);
		LOGD << "Size of the entire app/game obj.: sizeof(game) == " << sizeof(game);

		Main.exit_code = game.run(); // exit_code won't be set on exceptions!

		LOGI	<< "Profiling stats:\n"
			<< "------------------------------------------------------\n"
			<< "- Use Tracy (and build with `CFLAGS_=-DTRACY_ENABLE`)!\n"
			<< "------------------------------------------------------\n"
		;

//BUG("Oh, crap!");
//ABORT("Explicit abort!");
//cerr << "- ...but couldn't really?! WTF??? :-o ";

	} catch (const Szim::diag::FatalError&) { //!! Do sg. about this ugly low-level-internal-detail name!... :-/
	                                    //!! Well, it'll disappear when moving from this direct-main to an app-runner setup! :)
		// Message already delivered by FATAL_ERROR()...
	} catch (const runtime_error& x) {
		ERROR( ("`std::runtime_error` exception: "s + x.what()).c_str() );
	} catch (const exception& x) {
		ERROR( ("`std::exception`: "s + x.what()).c_str() );
	} catch (...) {
		ERROR("UNKNOWN EXCEPTION!");
	}
	return Main.exit_code;
}
