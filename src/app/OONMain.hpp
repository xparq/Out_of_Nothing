//!!
//!! DeSFMLize the rest (mostly SFML_WINDOW(), and direct Mouse access), and move to OONApp!
//!!

#ifndef WILIRTHG029828Y9VCY37829045YGCM4EDF
#define WILIRTHG029828Y9VCY37829045YGCM4EDF

#include "OON.hpp"
#include "OONMainDisplay_sfml.hpp"
#include "SAL/event/Input-fw.hpp"

namespace OON {

//============================================================================
class OONMain : public OONApp
//!
//! NOTE: A CRTP(-like) setup would break the compilation barrier between
//!	backend-specific and -agnostic code! :-/
//!
//!	template< class OONMain >
//!	class OONApp ...
{
protected:
	void process(const SAL::event::Input& event) override;

        OONMainDisplay_sfml oon_main_view_;

// C++ mechanics... (Only to init the SFML-specific oon_main_view_!)
public:
	OONMain(Szim::RuntimeContext& runtime);
}; // class OONMain

} // namespace OON

#endif // WILIRTHG029828Y9VCY37829045YGCM4EDF
