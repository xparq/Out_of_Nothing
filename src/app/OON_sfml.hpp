#ifndef _WILIRTHG029828Y9VCY37829045YGCM4EDF_
#define _WILIRTHG029828Y9VCY37829045YGCM4EDF_

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
#include "Szim/UI/HUDStream.hpp"

#include <utility> // std::unreachable

namespace OON {

//============================================================================
class OONApp_sfml : public OONApp
//!
//! NOTE: A CRTP impl. would break the compilation barrier between backend-specific
//!	and "pure" code! :-/
//!
//!	template< class AppSysImpl > // CRTP for backend-specifics
//!	class OONApp_sfml : private _internal::FUCpp_ViewHack, public OONApp
//!
{
//--------------------------------------------------------------------
// SFML-specific overrides...
//--------------------------------------------------------------------
protected:
	//!! DeSFMLize parts most of these & move to OONApp:
	void event_loop() override; // Uses the SFML Event stuff + sf::Window
	void update_thread_main_loop() override; // Uses sf::Window, sf::sleep
	void draw() const override; // Uses sf::Window

//------------------------------------------------------------------------
// C++ mechanics...
//------------------------------------------------------------------------
public:
	OONApp_sfml(const Szim::RuntimeContext& runtime, int argc, char** argv);

//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
        OONMainDisplay_sfml oon_main_view_;

}; // class OONApp_sfml

} // namespace OON

#endif // _WILIRTHG029828Y9VCY37829045YGCM4EDF_
