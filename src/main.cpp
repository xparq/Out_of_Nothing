#include "extern/Args.hpp"
#include "OON_sfml.hpp"

#include <string>
#include <cstdlib> // atoi
#include <iostream> // cout

#include "extern/iprof/iprof.hpp"
#include "extern/iprof/iprof.cpp" //! Better than fiddling with the Makefile!... ;)

using namespace std;

//!!...
#include "../out/commit_hash.inc"
//!!Also:
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
	if (args["moons"])
		game.add_bodies( ::atoi(args("moons").c_str()) -2 ); // 2 were pre-created...
	if (args["interact"])
		game.interact_all();
	if (args["snd-fx"]) {
		//!! Currently only this much we can do with --snd-fx=on/off:
		//!! This is the only control we have, assuming it's on by default...
		game.toggle_sound_fxs(); //!! game.switch_sound_fxs(args("snd-fx") == "on"));
	}
	if (args["zoom"]) {
		float factor = stof(args("zoom").c_str());
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
