//! Force-include this during build (e.g. with /FI), instead of relying on
//! manually doing so everywhere (or even more risky: "where applicable"...)
//!
//! To support not including this at all, the original #defines have been
//! negated, so now the defaults give the best experience.
//! (Also, only defining them here if they haven't been already, to allow
//! controlling them from the build.)

#ifndef _4059786V2MB67B5VB7I3C5_
#define _4059786V2MB67B5VB7I3C5_


#ifndef DISABLE_THREADS
//#define DISABLE_THREADS
#endif

#ifndef DISABLE_AUDIO
//#define DISABLE_AUDIO
#endif

#ifndef DISABLE_HUD
//#define DISABLE_HUD
#endif


#endif // _4059786V2MB67B5VB7I3C5_
