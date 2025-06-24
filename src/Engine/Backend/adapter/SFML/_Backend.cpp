#include "_Backend.hpp"

#include "Engine/diag/Log.hpp"

using namespace Szim;

//-------------------------------------
SFML_Backend_Props::SFML_Backend_Props(SimAppConfig& syscfg)
	: sfml_hci(syscfg)
{
}

//-------------------------------------
SFML_Backend::SFML_Backend(SimAppConfig& syscfg)
	: SFML_Backend_Props(syscfg)
	, Backend(
		sfml_clock,
		sfml_hci,
		sfml_audio
	)
{
	LOG << "SFML backend initialized.";
}

//-------------------------------------
SFML_Backend& SFML_Backend::use(SimAppConfig& syscfg)
{
	static SFML_Backend sfml_backend{syscfg};
	return sfml_backend;
}
