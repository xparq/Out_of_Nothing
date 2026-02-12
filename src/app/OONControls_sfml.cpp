#include "OONControls.hpp"

#include "OON.hpp" // OONApp.gui (App/Base would be enough, if it was declared as SimApp*, but OONApp is more future-proof.)
#include "Szim/UI.hpp"

//!! :-(((
#include "Szim/Core/Device/HCI/Keyboard/adapter/SFML/keycodes.hpp" // SFML -> SimApp keycode translation


/*!!
#include <cstring>
OONController::OONController()
{
// We have a virtual fn., so this is out:
//	std::memset(this, 0, sizeof(*this));
}
!!*/

namespace OON {

void OONController::update() //!! Check the cost of keystate(), calling the Win32 API several times!!!
{
	using namespace Szim::HCI;

	if (app_->gui.focused()) {
		return;
	}

//	PanUp    = keystate(W);
//	PanDown  = keystate(S);
//	PanLeft  = keystate(A);
//	PanRight = keystate(D);
	PanUp    = keystate(UP);
	PanDown  = keystate(DOWN);
	PanLeft  = keystate(LEFT);
	PanRight = keystate(RIGHT);

	PanFollow = keystate(SHIFT);
	PanLock   = keystate(SCROLL_LOCKED);

	ZoomIn   = keystate(NUMPAD_PLUS);
	ZoomOut  = keystate(NUMPAD_MINUS);

	ShowOrbits = keystate(LCTRL) && keystate(RCTRL);
	ShowDebug = keystate(RALT);

//	ThrustUp    = keystate(UP);
//	ThrustDown  = keystate(DOWN);
//	ThrustLeft  = keystate(LEFT);
//	ThrustRight = keystate(RIGHT);
	ThrustUp    = keystate(W);
	ThrustDown  = keystate(S);
	ThrustLeft  = keystate(A);
	ThrustRight = keystate(D);
	ThrustX     = -((decltype(ThrustX))ThrustLeft) + ((decltype(ThrustX))ThrustRight);
	ThrustY     = -((decltype(ThrustY))ThrustUp)   + ((decltype(ThrustY))ThrustDown);

	Chemtrail = keystate(SPACE);
	Shield    = keystate(LALT);
}

} // namespace OON