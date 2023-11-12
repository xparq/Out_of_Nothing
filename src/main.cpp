#include "extern/Args.hpp"
#include "OON_sfml.hpp"

#include "extern/iprof/iprof.hpp"
#include "extern/iprof/iprof.cpp" //! Better than fiddling with the Makefile!... ;)

#include <string>
#include <iostream> // cout
#include <algorithm> // min
#include <stdexcept> // exception, runtime_error

using namespace std;

#include "commit_hash.inc" //!! Must be placed somewhere on the INCLUDE path by the build proc.!
//!!Another "silent extern" hackery:
/*EXPORT*/ bool DEBUG_cfg_show_keycode = false;

//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
	Args args(argc, argv, {
	// Long options with 1 param. don't need to be defined:
	//	{"moons", 1}, // number of moons to start with
	// Short ones do, unfortunately (they're predicates by default, and don't have '=' to override):
		{"C", 1},
	});
	//auto exename = args.exename();
	if (args["?"] || args["h"] || args["help"]) {
		cout << "Usage: " << args.exename() << " [-V] [--moons=n] [-K]"
			<< endl;
		return 0;
	} else if (args["V"]) {
		cout << "Version: " << LAST_COMMIT_HASH << endl;
		return 0;
	} else if (args["K"]) {
		DEBUG_cfg_show_keycode = true;
	}

try {
	OON_sfml game(argc, argv);

	   if (args["bodies"]) {
		auto n = stoi(args("bodies")) - 2; // 2 have been pre-created!...
		game.add_bodies(n < 0 ? 0 : n); // Avoid possible overflow!
	}; if (args["interact"]) {
		game.interact_all();
	}; if (args["friction"]) {
		float f = stof(args("friction"));
		game.world().FRICTION = f;
	}; if (args["snd"]) {
		game.backend.audio.enabled(args("snd") != "off");
	}; if (args["zoom"]) {
		float factor = stof(args("zoom"));
		game.zoom(factor);
	}

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
//----------------------------------------------------------------------------
