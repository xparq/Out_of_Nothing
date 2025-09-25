#ifndef _WILIRTHG029828Y9VCY37829045YGCM4EDF_
#define _WILIRTHG029828Y9VCY37829045YGCM4EDF_

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
#include "Engine/UI/HUDStream.hpp"

#include <utility> // std::unreachable

namespace OON {

//============================================================================

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//! Sigh... This C++ hack is so sad... :-/
class OONApp_sfml;
namespace _internal {
	struct FUCpp_ViewHack {
		struct _oon_view_container {
			OONMainDisplay_sfml _oon_main_view;
			_oon_view_container(OONApp_sfml& app);
		} _oon_view;
		FUCpp_ViewHack(OONApp_sfml& app);
	};
}
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

class OONApp_sfml : private _internal::FUCpp_ViewHack, public OONApp
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
	void draw() override; // Uses sf::Window

//------------------------------------------------------------------------
// C++ mechanics...
//------------------------------------------------------------------------
public:
	OONApp_sfml(const Szim::RuntimeContext& runtime, int argc, char** argv);

//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:

	UI::HUDStream& ui_gebi(HUD_ID which) override;

}; // class OONApp_sfml

} // namespace OON

#endif // _WILIRTHG029828Y9VCY37829045YGCM4EDF_
