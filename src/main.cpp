#include "OON_sfml.hpp"

#include "extern/iprof/iprof.hpp"
#include "extern/iprof/iprof.cpp" //! Better than fiddling with the Makefile!... ;)

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
	Args args(argc, argv); // Just a prelim. "preview" :) parse for -h -V etc.
	if (args["?"] || args["h"] || args["help"]) {
		cout << "Usage: " << args.exename() << " [-V] [-C cfgfile] [...lots more undoc. yet, sorry!]\n";
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

try {
	OON_sfml game(argc, argv);
	game.run();

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

	cerr << "Profiling stats: " << IPROF_ALL_THREAD_STATS;

	return 0;
}
