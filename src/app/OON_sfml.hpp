#ifndef _WILIRTHG029828Y9VCY37829045YGCM4EDF_
#define _WILIRTHG029828Y9VCY37829045YGCM4EDF_

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
//!!Move to a proper polymorphic UI (e.g. myco):
#include "Engine/UI/hud_sfml.hpp"
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
// Data / Internals...
//------------------------------------------------------------------------
protected:

#ifndef DISABLE_HUDS
//!!	UI::HUD& ...;
	UI::HUD_SFML timing_hud;
	UI::HUD_SFML world_hud;
	UI::HUD_SFML view_hud;
	UI::HUD_SFML object_hud;
	UI::HUD_SFML help_hud;
	UI::HUD_SFML debug_hud;

	UI::HUD& ui_gebi(HUD_ID which) override { switch (which) {
		case TimingStats: return timing_hud;
		case WorldData:   return world_hud;
		case ViewData:    return view_hud;
		case ObjMonitor:  return object_hud;
		case HelpPanel:   return help_hud;
		case Debug:       return debug_hud;
		default: std::unreachable(); // c++23 only; and this will be c++999: [[unreachable]]
			//return help_hud; // Dummy, to shut up some pre-c++23 compiler warnings
	}}
#endif
}; // class OONApp_sfml

} // namespace OON

#endif // _WILIRTHG029828Y9VCY37829045YGCM4EDF_
