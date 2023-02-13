#include "Args.hpp"
#include "OON_sfml.hpp"

#include <string>
#include <cstdlib> // atoi
#include <iostream> // cout

using namespace std;

//!!...
#include "../out/commit_hash.inc"

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
		cout << "Usage: " << args.exename() << " [-V] [--moons=n]" << endl;
		return 0;
	}
	if (args["V"]) {
		cout << "Version: " << LAST_COMMIT_HASH << endl;
		return 0;
	}

	OON_sfml game;/*({
		.moons = args("moons")
	});*/
	if (args["moons"]) {
		game.add_bodies( ::atoi(args("moons").c_str()) -2 ); // 2 were pre-created...
	}

	try {
		game.run();
	} catch (...) {
		cerr << "UNHANDLED EXCEPTION!\n";
		return -1;
	}

	return 0;
}
//----------------------------------------------------------------------------
