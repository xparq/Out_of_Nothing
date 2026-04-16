#include "OONControls.hpp"

#include "OON.hpp" // OONApp.gui (App/Base would be enough, if it was declared as SimApp*, but OONApp is more future-proof.)
#include "Szim/UI.hpp"
#include "Szim/Core/Device/HCI/Keyboard.hpp" //!! Make this less cumbersome!


/*!!
#include <cstring>
OONController::OONController()
{
// We have a virtual fn., so this is out:
//	std::memset(this, 0, sizeof(*this));
}
!!*/

namespace OON {

void OONController::update(Szim::Core::SimApp& app) //!override //!! Check the cost of keystate(), calling the Win32 API several times!!!
{
	if (app.gui.focused()) return; //!! Should be done by the caller (which should be app.poll_controls(), called by the engine)!

	//!!
	//!! The cringy static keystate() should be accessed via app.rt.hci instead!
	//!!

	//!! Merge these, finally!...:
	using namespace Szim::HCI;
	using enum SAL::event::VKey;

//	PanUp    = keystate(W);
//	PanDown  = keystate(S);
//	PanLeft  = keystate(A);
//	PanRight = keystate(D);
	PanUp    = keystate(Up);
	PanDown  = keystate(Down);
	PanLeft  = keystate(Left);
	PanRight = keystate(Right);

	PanFollow = keystate(Shift);

	PanLock   = keystate(Scroll_Locked); //!! A virtual key state! Make that self-documenting!

	ZoomIn   = keystate(NumpadPlus);
	ZoomOut  = keystate(NumpadMinus);

	ShowOrbits = keystate(LCtrl) && keystate(RCtrl);
	ShowDebug = keystate(RAlt);

//	ThrustUp    = keystate(Up);
//	ThrustDown  = keystate(Down);
//	ThrustLeft  = keystate(Left);
//	ThrustRight = keystate(Right);
	ThrustUp    = keystate(W);
	ThrustDown  = keystate(S);
	ThrustLeft  = keystate(A);
	ThrustRight = keystate(D);
	ThrustX     = -((decltype(ThrustX))ThrustLeft) + ((decltype(ThrustX))ThrustRight);
	ThrustY     = -((decltype(ThrustY))ThrustUp)   + ((decltype(ThrustY))ThrustDown);

	Chemtrail = keystate(Space);
	Shield    = keystate(LAlt);
}

} // namespace OON