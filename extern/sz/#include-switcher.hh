#ifndef _VV980N7X08T2X0L83Z5867CMV8BN_
#define _VV980N7X08T2X0L83Z5867CMV8BN_

// Works with both the legacy MSVC preproc, and also with /ZC:preprocessor
// Bonus: `include SWITCH(X)` won't compile if X is undefined or has an
// unsupported value.

/*
//-------------------------------------------------------
// Config:
//
// MYPROP
//
#define MYPROP one
//#define MYPROP two
*/

//-------------------------------------------------------
// Conditional code:
//
// For each supported case X of PROP, the desired conditional code is in
//
//	cfg/PROP_dispatch/X
//
// (E.g. #define MYPROP default --> cfg/MYPROP_dispatch/default
//

//-------------------------------------------------------
// Switcher:
//

// Switcher cfg.:
/*
#ifndef SWITCH_TAG_PRE
#define SWITCH_TAG_PRE
#endif
#ifndef SWITCH_TAG_POST
#define SWITCH_TAG_POST
#endif
*/
#ifndef SWITCH_PATTERN_ // User-defined "callback", with an example default here:
#define SWITCH_PATTERN_(tagged_name, value) _CONCAT__(tagged_name, value.h)
#endif
#ifndef _SWITCH_PATTERN // User-defined "callback", with an example default here:
#define _SWITCH_PATTERN(value, tagged_name) _CONCAT__(value, tagged_name.h)
#endif

#ifndef SWITCHED_PATTERN // User-defined "callback", with an example default here:
#define SWITCHED_PATTERN(tagged_name, value, header) tagged_name/value/header
#endif


//#define _CONCAT__(A,B) _CONCAT_IMPL__(A,B)
//#define _CONCAT_IMPL__(A,B) A##B
#define _STRVAL__(A) _STR__(A)
#define _STR__(A) #A

//#define _FW__(X) X
//#define _KEEP__(X, ...) X##__VA_ARGS__
	//!! Doesn't help: the optionals can't be empty, or else X would still be expanded!

//!! This isn't enough for default params: if $val is undefined, it would still
//!! arrive here, as a literal name! :-/
//#define _DEF__(val) _DEF2__(val, default)
//#define _DEF2__(val, ...) val

// The superfluous ## concat. is the trick that makes it work (prevents expansion of $name)!...
// (Note: if $name (say MyProp) is not defined, it will be passed as a literal symbol (i.e. MyProp)!
// That, at least, can be exploited as some crude/lame default-case kludge...)
#define SWITCH(name)   _STRVAL__(SWITCH_PATTERN_(name##_,name))
//!!#define SWITCH(name, ...) _SWITCH__(__VA_ARGS__##name, name) //!! Doesn't help, see _KEEP__()!

#define SWITCHED(switch, header)   _STRVAL__(SWITCHED_PATTERN(switch##_,switch,header))


#define SWITCH_(name)  _STRVAL__(SWITCH_PATTERN_(name##_,name))
#define _SWITCH(name)  _STRVAL__(_SWITCH_PATTERN(name,_##name))

#define SWITCH_POSTFIXED(name, post) _STRVAL__(SWITCH_PATTERN_(name##post,name))
#define PREFIXED_SWITCH(pre, name)  _STRVAL__(_SWITCH_PATTERN(name,pre##name))

// No amount of trial-and-error and investigation helped me avoid that _,
// or at least replace it with something pre-configured (i.e. not a macro arg)...:
//#define oldSWITCH_(name) _SWITCH__(_CONCAT__(_CONCAT__(SWITCH_TAG_PRE,name##_), SWITCH_TAG_POST), _DEF__(name)) // This superfluous ## concat. is the trick that makes it work, actually!
//#define old_SWITCH(name) _SWITCH__(_CONCAT__(_CONCAT__(SWITCH_TAG_PRE,_##name), SWITCH_TAG_POST), _DEF__(name)) // This superfluous ## concat. is the trick that makes it work, actually!


/*
//-------------------------------------------------------
// Client code, using it:
//

#define SWITCH_PATTERN(tagged_name, value) xcfg-tagged_name##value.h
#include "switcher.hh"

//#include SWITCH(MYPROP)

#include <iostream>
int main()
{
//	std::cout << config_myprop << ", from: " << SWITCH(MYPROP);
	std::cout << SWITCH_(MYPROP) << '\n';
	std::cout << _SWITCH(MYPROP) << '\n';
}
*/
#endif // _VV980N7X08T2X0L83Z5867CMV8BN_
