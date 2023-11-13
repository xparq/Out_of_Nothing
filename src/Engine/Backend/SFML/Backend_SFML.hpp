﻿#ifndef _C3748962874B647YRTH56_
#define _C3748962874B647YRTH56_

#include "Engine/Backend.hpp"
// Adapters:
#include "Clock_SFML.hpp"
#include "HCI_SFML.hpp" // sf::Window, events etc.
#include "Audio_SFML.hpp"

//!!TBD: These are not direct SFML deps., only they themselves depend on it...
//!!??   How to deal with this cleanly?
//!!#include "sfw/GUI.hpp"     //!! Not yet at its appropriate location!
//!!#include "UI/hud_sfml.hpp" //!! Not yet at its appropriate location! (Should be in sfw!)

//!!Which subsystem should this go to, if any?
//!!#include <SFML/System/Sleep.hpp>

namespace Szim {

struct SFML_Backend_Props // -> base-from-member C++ idiom
{
	SFML_Clock sfml_clock;
	SFML_HCI   sfml_hci;
	SFML_Audio sfml_audio;

//!!... Back in SimApp.hpp, directly for now:
//!!	sfw::GUI& gui; //!! like a sore thumb...

	SFML_Backend_Props(const Config& syscfg);
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
	sf::RenderWindow& SFML_window() { return sfml_hci.window(); }

	//------------------------------------------------------------------------
	// Plumbing...
	//------------------------------------------------------------------------
public:
	static SFML_Backend& use(const Config& syscfg);
//!!	static SFML_Backend& get() { return use(Config{}); } //! Dummy cfg to be ignored
private:
	SFML_Backend(const Config& syscfg);
}; // class SFML_Backend

} // namespace Szim
#endif // _C3748962874B647YRTH56_
