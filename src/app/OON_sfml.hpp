#ifndef WILIRTHG029828Y9VCY37829045YGCM4EDF
#define WILIRTHG029828Y9VCY37829045YGCM4EDF

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
#include "SAL/event/Input.hpp"

namespace OON {

//============================================================================
class OONApp_sfml : public OONApp
//!
//! NOTE: A CRTP(-like) setup would break the compilation barrier between
//!	backend-specific and -agnostic code! :-/
//!
//!	template< class OONApp_sfml >
//!	class OONApp ...
//!
{
//--------------------------------------------------------------------
// Overrides with SFML-specific impl...
protected:
	//!! "DeSFMLize & move to OONApp:
	void process(const SAL::event::Input& event) override;
	void draw() const override; // Uses sf::Window

//------------------------------------------------------------------------
// Data...
protected:

        OONMainDisplay_sfml oon_main_view_;

//------------------------------------------------------------------------
// C++ mechanics... (Only to init oon_main_view_!)
public:
	OONApp_sfml(Szim::RuntimeContext& runtime);

}; // class OONApp_sfml

} // namespace OON

#endif // WILIRTHG029828Y9VCY37829045YGCM4EDF
