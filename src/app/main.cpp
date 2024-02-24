// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"
#include "extern/Tracy/public/TracyClient.cpp"

#include "OON_sfml.hpp"

#include <iostream>
	using std::cout, std::cerr;
#include <stdexcept>
	using std::exception, std::runtime_error;


// LAST_COMMIT_HASH is defined here:
#include "commit_hash.inc"
	//!! Must be placed somewhere on the INCLUDE path by the build proc.!


//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
	using namespace OON;

	//!! Sad kludge util #348...:
	Args args(argc, argv); // Just for -h -V etc.
	if (args["?"] || args["h"] || args["help"]) {
		//!!game.show_cmdline_help();
		OONApp_sfml::show_cmdline_help(args);
		return 0;
	} else if (args["V"]) {
		cout
			<< "Version: " << LAST_COMMIT_HASH
//!! The build proc. should just send the VDIR tag in a macro! -> #254
#ifdef DEBUG
			<< "-DEBUG"
#endif
#ifdef SFML_STATIC
			<< "-SFML_STATIC"
#endif
			<< '\n';
		return 0;
	}


	int exit_code = 0;
	try {
		OONApp_sfml game(argc, argv);
cerr << "DBG> Size of the app (game) obj.: "<< sizeof(game) << '\n';

		exit_code = game.run();

	} catch (runtime_error& x) {
		cerr << "- ERROR: " << x.what() << '\n';
		return -1;
	} catch (exception& x) {
		cerr << "- EXCEPTION: " << x.what() << '\n';
		return -1;
	} catch (...) {
		cerr << "- UNKNOWN EXCEPTION!\n";
		return -1;
	}

	cerr	<< "Profiling stats:\n"
		<< "------------------------------------------------------\n"
		<< "- Use Tracy (and build with `CFLAGS_=-DTRACY_ENABLE`)!\n"
		<< "------------------------------------------------------\n"
	;

	cerr << "DBG> main() returning: " << exit_code << '\n';

	return exit_code;
}
