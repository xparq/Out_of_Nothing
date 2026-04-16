#ifndef X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V
#define X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V

#include "Szim/Core/Device/HCI/VirtualController.hpp"

namespace OON {

class OONApp;

struct OONController : public Szim::Core::VirtualController
//!! All for polling only, no async triggers yet!
//!! This also means that Toggle values make no sense...
{
	// View control
	PushButton ZoomIn{};
	PushButton ZoomOut{};
	PushButton PanLeft{};
	PushButton PanRight{};
	PushButton PanUp{};
	PushButton PanDown{};

	PushButton PanFollow{};
	Toggle PanLock{};

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

	void update(Szim::Core::SimApp& app) override; // Implemented in the backend- (currently SAL-) specific part of the app
};

} // namespace OON

#endif // X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V
