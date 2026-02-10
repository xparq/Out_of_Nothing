#ifndef _WILIRTHG029828Y9VCY37829045YGCM4EDF_
#define _WILIRTHG029828Y9VCY37829045YGCM4EDF_

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
#include "Szim/UI/HUDStream.hpp"
#include "SAL/event/Input.hpp"

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
// Overrides with SFML-specific impl...
//--------------------------------------------------------------------
protected:
	//!! "DeSFMLize & move to OONApp:
	void draw() const override; // Uses sf::Window
	//!! Moved to O2N, finally:
	//!!void event_loop() override; // Uses the SFML Event stuff + sf::Window
	//!!void update_loop() override; // Uses sf::Window, sf::sleep

//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
	void process(const SAL::event::Input& event) override;

//------------------------------------------------------------------------
// C++ mechanics...
//------------------------------------------------------------------------
public:
	OONApp_sfml(const Szim::RuntimeContext& runtime);

//------------------------------------------------------------------------
// Data...
//------------------------------------------------------------------------
protected:

        OONMainDisplay_sfml oon_main_view_;

}; // class OONApp_sfml

} // namespace OON

#endif // _WILIRTHG029828Y9VCY37829045YGCM4EDF_
