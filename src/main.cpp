#include "cfg.h"
#include "engine_sfml.hpp"

#include <iostream> // cerr
using namespace std;

//============================================================================
int main(/*int argc char* argv[]*/)
//============================================================================
{
	Engine_SFML engine;
cerr << "TRACE - engine created.\n";

	engine.run();

	return 0;
}
