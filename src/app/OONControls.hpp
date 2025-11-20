#ifndef _X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V_
#define _X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V_

#include "sz/toggle.hh"
#include <cstdint>

#include "Szim/HCI/VirtualController.hpp"

namespace OON {

class OONApp;

struct OONController : public Szim::VirtualController
//!! All for polling only, no async triggers yet!
//!! This also means that Toggle values make no sense
{
	// View control
	PushButton ZoomIn{};
	PushButton ZoomOut{};
	PushButton PanLeft{};
	PushButton PanRight{};
	PushButton PanUp{};
	PushButton PanDown{};

	PushButton PanFollow{};
	HardToggle PanLock{};

	PushButton ShowOrbits{};
	PushButton ShowDebug{};

	// Player movement
	//!! Could/should be a joystick levels instead!
//!!/*!! OBSOLETE:
	Level8u ThrustLeft{};
	Level8u ThrustRight{};
	Level8u ThrustUp{};
	Level8u ThrustDown{};
//!!!!*/
	Level8 ThrustX{};
	Level8 ThrustY{};

	// Player actions
	PushButton Chemtrail{};
	PushButton Shield{};

	// Meta/Admin actions
//!!...	LatchedToggle Pause;

	void update() override; // Implemented in the backend-specific part of the app!

	OONController(OONApp* app) : app_(app) {}
private:
	OONApp* app_;
};

} // namespace OON

#endif // _X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V_
