#ifndef _WILIRTHG029828Y9VCY37829045YGCM4EDF_
#define _WILIRTHG029828Y9VCY37829045YGCM4EDF_

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
#include "Szim/UI/HUDStream.hpp"

#include <utility> // std::unreachable

namespace OON {

//============================================================================

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//! Sigh... This is a sad C++ "base-from-member" hack to allow constructing
//! the main view before the app base (OONApp), to pass it to its ctor (which
//! in turns passes it along to the engine's app base (SimApp))... :-/
//! (Note listing FUCpp_ViewHack before OONApp in the decl. of OONApp_sfml.
//! And the nested double structs were used for explicit scoping/quarantining
//! that hitchhiking member, and possibly making room for other similar ones.)
class OONApp_sfml;
namespace _internal {
	struct  FUCpp_ViewHack {
	        FUCpp_ViewHack(OONApp_sfml& app);
		struct  _bfm_container_{
		        _bfm_container_(OONApp_sfml& app);
		        OONMainDisplay_sfml _oon_main_view;
		} _bfm_;
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

}; // class OONApp_sfml

} // namespace OON

#endif // _WILIRTHG029828Y9VCY37829045YGCM4EDF_
