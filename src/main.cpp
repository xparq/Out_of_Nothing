#include "engine_sfml.hpp"

#include <iostream> // cerr
using namespace std;


//!!...
extern
#include "../out/commit_hash.inc"

//============================================================================
int main(/*int argc char* argv[]*/)
//============================================================================
{
cerr << "Last commit: " << LAST_COMMIT_HASH << endl;

	Engine_SFML engine;

	engine.run();

	return 0;
}
