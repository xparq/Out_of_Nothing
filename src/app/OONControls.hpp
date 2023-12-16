#ifndef _X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V_
#define _X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V_

#include "sz/toggle.hh"
#include <cstdint>

//!!??#include "Engine/.../HCI.hpp"
//!!#include "UI??/Input.hpp"
namespace Szim {

class VirtualController
{
// - Polled/sampled signal levels for continuous inputs
// - One-shot interrupting, real-time) input events
// - One-shot non-interrupting inputs
//	-> either converted to pollable "stateful" levels,
//         or queued as classic ("edge") events!
///
// Buffered inputs should probably not even be inside the Controller?
// Or: they can be deserialized, and multiple repeated keypresses only
// registered once, like elevator call buttons: the elevator won't come
// as many times as the button has been pressed, only once. :)
// This could be very handy to avoid unwanted buffering of delayed inputs
// when the event queue is congested!
//
// ...

public:
	//!!
	//!! All these are to be polled, rather than processed asynchronously
	//!! -- as there are no callbacks for them...
	//!!
	using PushButton = bool; // Active while being held
	using Level16 = int16_t;
	using Level8  = int8_t;
	using Level16u = uint16_t;
	using Level8u  = uint8_t;

	struct ActionRequest //!! Find a better name to reflect its "call for action"
	                     //!! semantics: "multiple call -> one action"!
	//!!??
	//!!?? This is a generalized pollable state, Or unify this with polled inputs
	//!!??

	// Latched events (typically deserialized & unbuffered from an event queue)
	// Repeated (more-or-less?) identical inputs are collapsed into a single
	// pending event by the low-level input controller (the last overriding
	// the previous, rather than keeping the first and ignoring the rest).
	//
	// Apps should derive their specific versions with descriptive names
	// any cargo data added etc.
	{
		uint8_t count; // 0: idle, >0: pending (and requested 'count' times)
		void set() { if (!++count) --count; } // Don't overflow!
		operator bool() { return count; }
	};

	using HardToggle = sz::Toggle; // Level-driven, pollable

	//!! Two kinds of toggles seem to make sense:
	//!! - "Hard" (or "async" or "explicit" or "absolute"):
	//!!    Those with their own state indicator, like the Lock keys,
	//!!    so the meaning of repeated inputs is alywas clear.
	//!!    They're just pushbuttons with a brick on them. :)
	//!!    -> Can be polled directly & straightforwardly.
	//!! - "Soft" (or "sync" or "implicit or "relative"):
	//!!    These are just a state in software, where repeated inputs may
	//!!    not have a clear effect, and the intent might get out of sync
	//!!    with the actually registered state if processing is delayed.
	//!!    These should therefore not be queued as separate events, but
	//!!    go through latching, collapsing all the "random" flippings
	//!!    into one single pending event -- so e.g. pressing Pause
	//!!    multiple times, or Tab for toggling full interactions, while
	//!!    the event queue is jammed, will a) not result in a hard-to-
	//!!    remember effective state for each edge, so b) should not be
	//!!    buffered.
	//!!!!  BUT TEST THIS, as in some use cases it could still be preferable
	//!!!!  to register each individual edge, rather than losing some,
	//!!!!  assuming that they are just redundant impatience-repeats!...
	//!!!!  Maybe latching should kick in only during congestion?!
	//!!
	//!!	ALSO:
	//!!
	//!!	It's possible that all this above is only relevant to the
	//!!	low-level actual input processing side of it, and then this
	//!!	presentation side of can always be "normalzied" into a simple
	//!!	pollable toggle state. (But then again: processing ON-OFF-ON
	//!!    sequences are only sometimes acceptable, so "always" and
	//!!    "simple" already can't be coupled unconditionally...)

	/*!!!!	AND:

		Sync/async may not map cleanly to the THREE! :) processing
		states of: unbuffered (direct-polled) / latched / buffered!...
	!!*/

//!!...	struct LatchedToggle : ActionRequest {};

	virtual void update() = 0; // Call this from your system-specific input polling loop!
};
} // namespace Szim

//!!namespace OON {

struct OONController : public Szim::VirtualController
//!! All for polling only, no async triggers yet!
//!! This also means that Toggle values make no sense
{
	// View control
	PushButton ZoomIn;
	PushButton ZoomOut;
	PushButton PanLeft;
	PushButton PanRight;
	PushButton PanUp;
	PushButton PanDown;

	PushButton PanFollow;
	HardToggle PanLock;

	PushButton ShowOrbits;

	// Player movement
	//!! Could/should be a joystick levels instead!
//!!/*!! OBSOLETE:
	Level8u ThrustLeft;
	Level8u ThrustRight;
	Level8u ThrustUp;
	Level8u ThrustDown;
//!!!!*/
	Level8 ThrustX;
	Level8 ThrustY;

	// Player actions
	PushButton Chemtrail;
	PushButton Shield;

	// Meta/Admin actions
//!!...	LatchedToggle Pause;

	void update() override; // Implemented in the backend-specific part of the app!
};

//} // namespace OON

#endif // _X789N65B78CXM45VB60N45I8UBTYWHBIOUSGHB7C476V_
