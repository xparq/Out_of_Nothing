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

void OONController::update() //!! Check the cost of keystate(), calling the Win32 API several times!!!
{
	using namespace UI;

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

	ShowOrbits = keystate(ALT) && keystate(CTRL);

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
