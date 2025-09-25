#ifndef _C3748962874B647YRTH56_
#define _C3748962874B647YRTH56_

#include "Engine/Backend.hpp"
// Adapters:
#include "_Clock.hpp"
#include "_HCI.hpp" // sf::Window, events etc.
#include "_Audio.hpp"

//!!TBD: This is not a direct SFML dep., just that itself depends on it...
//!!#include "myco/GUI.hpp"     //!! Not yet at its appropriate location!

//!!Which subsystem should this go to, if any?
//!!#include <SFML/System/Sleep.hpp>

namespace Szim {

struct SFML_Backend_Props // -> base-from-member C++ idiom
{
	Time::SFML_Clock sfml_clock;
	SFML_HCI   sfml_hci;
	SFML_Audio sfml_audio;

//!!... Back in SimApp.hpp, directly for now:
//!!	myco::GUI& gui; //!! like a sore thumb...

	SFML_Backend_Props(SimAppConfig& syscfg);
};

class SFML_Backend : private SFML_Backend_Props, public Backend
{
	//------------------------------------------------------------------------
	// Szim API implementation (overrides)...
	//------------------------------------------------------------------------
public:

	//------------------------------------------------------------------------
	// SFML-SPECIFIC HELPERS...
	//------------------------------------------------------------------------
	sf::RenderWindow& SFML_window() { return sfml_hci.SFML_window(); }

	//------------------------------------------------------------------------
	// Plumbing...
	//------------------------------------------------------------------------
public:
	static SFML_Backend& use(SimAppConfig& syscfg);
//!!	static SFML_Backend& get() { return use(Config{}); } //! Dummy cfg to be ignored
private:
	SFML_Backend(SimAppConfig& syscfg);
}; // class SFML_Backend

} // namespace Szim
#endif // _C3748962874B647YRTH56_
