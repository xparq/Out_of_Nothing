#include "OONControls.hpp"

#include "UI/adapter/SFML/keycodes.hpp" // SFML -> SimApp keycode translation

/*!!
#include <cstring>
OONController::OONController()
{
// We have a virtual fn., so this is out:
//	std::memset(this, 0, sizeof(*this));
}
!!*/

void OONController::update()
{
	using namespace UI;

	PanUp    = keystate(W);
	PanDown  = keystate(S);
	PanLeft  = keystate(A);
	PanRight = keystate(D);

	PanFollow = keystate(SHIFT);
	PanLock   = keystate(SCROLL_LOCKED);

	ZoomIn   = keystate(NUMPAD_PLUS);
	ZoomOut  = keystate(NUMPAD_MINUS);

	ThrustUp    = keystate(UP);
	ThrustDown  = keystate(DOWN);
	ThrustLeft  = keystate(LEFT);
	ThrustRight = keystate(RIGHT);
	ThrustX     = -((decltype(ThrustX))ThrustLeft) + ((decltype(ThrustX))ThrustRight);
	ThrustY     = -((decltype(ThrustY))ThrustUp)   + ((decltype(ThrustY))ThrustDown);

	Chemtrail = keystate(SPACE);
	Shield    = keystate(RCTRL);
}
