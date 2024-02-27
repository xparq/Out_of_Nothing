#ifndef _4059786V2MB67B5VB7I3C5_
#define _4059786V2MB67B5VB7I3C5_

//! Force-include this during build (e.g. with /FI), instead of relying on
//! manually doing so everywhere (or even more risky: "where applicable"...)
//!
//! To support not including this at all (albeit some options would have to
//! be defined somewhere else then!), the original ENABLE_ flags have been
//! negated, so missing definitions by defaults give the best experience.
//! (Also, only defining them here if they haven't been already, to allow
//! controlling them via the build.)

//----------------------------------------------------------------------------
// System (Backend) Interface...
//
// (These still must be defined somewhere, so we can as well just do it here...
// If this file is not being included (and no other defs.), there would be
// compilation errors anyway, so no worries.)

#ifndef  BACKEND
# define BACKEND SFML
#endif

#ifndef  GFX_BACKEND
# define GFX_BACKEND    BACKEND
#endif

#ifndef  AUDIO_BACKEND 
# define AUDIO_BACKEND  BACKEND
#endif

#ifndef  INPUTS_BACKEND
# define INPUTS_BACKEND BACKEND
#endif

#ifndef  SYSTEM_BACKEND
# define SYSTEM_BACKEND BACKEND
#endif


//----------------------------------------------------------------------------
// Features On/Off...
//
#ifndef DISABLE_THREADS
//# define DISABLE_THREADS
#endif

#ifndef DISABLE_AUDIO
//# define DISABLE_AUDIO
#endif

#ifndef DISABLE_HUD
//# define DISABLE_HUD
#endif

#ifndef DISABLE_FULL_INTERACTION_LOOP
//# define DISABLE_FULL_INTERACTION_LOOP
#endif


//----------------------------------------------------------------------------
// Misc...
//
#define DEFAULT_MODEL_NUMBER_TYPE double


#endif // _4059786V2MB67B5VB7I3C5_
