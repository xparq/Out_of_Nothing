#include "extern/Args.hpp"
#include "OON_sfml.hpp"

#include <string> // stoi
#include <iostream> // cout
#include <algorithm> // min

#include "extern/iprof/iprof.hpp"
#include "extern/iprof/iprof.cpp" //! Better than fiddling with the Makefile!... ;)

using namespace std;

#include "commit_hash.inc" //!! Must be placed somewhere on the INCLUDE path by the build proc.!
//!!Another "silent extern" hackery:
bool DEBUG_cfg_show_keycode = false;

//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
	Args args(argc, argv, {
	// Options with 1 param. don't need to be defined:
	//	{"moons", 1}, // number of moons to start with
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

	OON_sfml game;/*({
		.moons = args("moons"),
		.interat = ...,
	});*/
	if (args["bodies"]) {
		auto n = stoi(args("bodies")) - 2; // 2 have been pre-created!...
		game.add_bodies(n < 0 ? 0 : n); // Avoid possible overflow!
	} if (args["interact"]) {
		game.interact_all();
	} if (args["friction"]) {
		float f = stof(args("friction"));
		game.get_world().FRICTION = f;
	} if (args["loopcap"]) {
		game.iterations.max(stoul(args("loopcap")));
	} if (args["snd"]) {
		game.audio.enabled(args("snd") != "off");
	} if (args["zoom"]) {
		float factor = stof(args("zoom"));
		game.zoom(factor);
	}

	try {
		game.run();
	} catch (...) {
		cerr << "UNHANDLED EXCEPTION!\n";
		return -1;
	}

	cerr << "Profiling stats: " << IPROF_ALL_THREAD_STATS;

	return 0;
}
//----------------------------------------------------------------------------
