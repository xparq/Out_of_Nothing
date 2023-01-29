//! Force-include this during build (e.g. with /FI), instead of relying on
//! manually doing so everywhere (or even more risky: "where applicable"...)
//!
//! To support not including this at all, the original #defines have been
//! negated, so now the defaults give the best experience.
//! Also, only define them if they aren't already, to allow external control
//! via build.

#ifndef __CFG_H_
#define __CFG_H_


#ifndef DISABLE_THREADS
//#define DISABLE_THREADS
#endif

#ifndef DISABLE_AUDIO
//#define DISABLE_AUDIO
#endif

#ifndef DISABLE_HUD
//#define DISABLE_HUD
#endif


#endif // __CFG_H_
