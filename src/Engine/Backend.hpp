#ifndef _VIY6OUAOIN4378G135Y_
#define _VIY6OUAOIN4378G135Y_

#include "SimAppConfig.hpp"
#include "Clock.hpp"
#include "HCI.hpp"
#include "Audio.hpp"

//!!#include <memory> // unique_ptr

namespace Szim {

class Backend // Abstract Backend Base
{
//!!	using CLockPtr = std::unique_ptr<Clock>;
//!!	using HCIPtr = std::unique_ptr<HCI>;
//!!	using AudioPtr = std::unique_ptr<Audio>;
//!!	...
//!! This would be too much, I think...

public:
	//------------------------------------------------------------------------
	// API...
	//------------------------------------------------------------------------

	// Directly accessible subsystems
	// (Initialized by the real (derived) backend ctor.)
	//!! Could also be implemented via virtual accessors, like clock(), but
	//!! that would also require a separate polymorphic (covariant) class pair! :-/
	Time::Clock& clock;
	HCI&   hci;
	Audio& audio;

protected:
	//! This lame extra ctor is needed to init the member references, because
	//! I think it can't be done directly by the derived class.
	//!!DBLCHK:
	//!!?? - Still needed with an abstract base?
	//!!?? - GCC/CLANG?
	Backend(
		Time::Clock& system_clock,
		HCI&   HCI_subsystem,
	        Audio& audio_subsystem
	) :
		clock(system_clock),
		hci  (HCI_subsystem),
	        audio(audio_subsystem)
	{}

	virtual ~Backend() = default;
}; // class Backend

} // namespace Szim

#endif // _VIY6OUAOIN4378G135Y_
