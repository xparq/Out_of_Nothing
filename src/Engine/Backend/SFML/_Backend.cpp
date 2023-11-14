#include "_Backend.hpp"

#include <iostream>
	using std::cerr;

using namespace Szim;

//-------------------------------------
SFML_Backend_Props::SFML_Backend_Props(const Config& syscfg)
	: sfml_hci(syscfg)
{
}

//-------------------------------------
SFML_Backend::SFML_Backend(const Config& syscfg)
	: SFML_Backend_Props(syscfg)
	, Backend(
		sfml_clock,
		sfml_hci,
		sfml_audio
	)
{
cerr << "Hi. SFML backend initialized.\n";
}

//-------------------------------------
SFML_Backend& SFML_Backend::use(const Config& syscfg)
{
	static SFML_Backend sfml_backend{syscfg};
	return sfml_backend;
}
