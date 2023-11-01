#include "extern/Args.hpp"
#include "OON_sfml.hpp"

#include <string> // stoi, stoul
#include <iostream> // cout
#include <algorithm> // min

#include "extern/iprof/iprof.hpp"
#include "extern/iprof/iprof.cpp" //! Better than fiddling with the Makefile!... ;)

using namespace std;

#include "commit_hash.inc" //!! Must be placed somewhere on the INCLUDE path by the build proc.!
//!!Another "silent extern" hackery:
/*EXPORT*/ bool DEBUG_cfg_show_keycode = false;

//============================================================================
int main(int argc, char* argv[])
//============================================================================
{
try {
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

	/*!! Normalize the config. process:
	[ ] pass it all in one go to the game
	[ ] preferably just the config file name!
		[ ] SimApp should have the cfg loader already;
			[ ] calling down to the real app impl. (OON)
			    for any non-generic (i.e.: unknown) option (-> inih, too)
	!!*/
	string cfg = args("cfg");
	if (cfg.empty()) cfg = args("C"); // OK if still empty
	OON_sfml game(cfg.c_str());/*({
		.moons = args("moons"),
		.interat = ...,
	});*/

#define or
	if (args["bodies"]) {
		auto n = stoi(args("bodies")) - 2; // 2 have been pre-created!...
		game.add_bodies(n < 0 ? 0 : n); // Avoid possible overflow!
	} or if (args["interact"]) {
		game.interact_all();
	} or if (args["friction"]) {
		float f = stof(args("friction"));
		game.get_world().FRICTION = f;
	} or if (args["loopcap"]) {
		auto loopcap = unsigned long(-1); // no limit
		try { loopcap = stoul(args("loopcap")); } catch(...) {
			cerr << "- WRNING: --loopcap ignored! Must be a valid positive int.\n";
		}
		game.iterations.max(loopcap);
	} or if (args["snd"]) {
		game.audio.enabled(args("snd") != "off");
	} or if (args["zoom"]) {
		float factor = stof(args("zoom"));
		game.zoom(factor);
	}
#undef or

	game.run();

} catch (...) {
	cerr << "- UNHANDLED EXCEPTION!\n";
	return -1;
}

	cerr << "Profiling stats: " << IPROF_ALL_THREAD_STATS;

	return 0;
}
//----------------------------------------------------------------------------
