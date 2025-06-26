#ifdef ERROR
// We're kinda losing against wingdi.h here...
# ifndef _MSC_VER
#  warning "Oh no, someone beat us to it: ERROR is already taken! ;) But never fear, let's just #undef it like a boss..."
# else
#  define _sz__STR_2(x) #x
#  define _sz__STR(x) _sz__STR_2(x)
#  pragma message( "Warning: Oh no, someone beat us to it: ERROR is already taken! ;) But never fear, let's just #undef it like a boss..." \
	" at " __FILE__ ":" _sz__STR(__LINE__)  )
#  undef _sz__STR
#  undef _sz__STR_2
# endif // _MSC_VER
#
# undef ERROR
#
#endif

#ifdef WARNING
# ifndef _MSC_VER
#  warning "Oh no, someone beat us to it: WARNING is already taken! ;) But never fear, let's just #undef it like a boss..."
# else
#  define _sz__STR_2(x) #x
#  define _sz__STR(x) _sz__STR_2(x)
#  pragma message( "Warning: Oh no, someone beat us to it: WARNING is already taken! ;) But never fear, let's just #undef it like a boss..." \
	" at " __FILE__ ":" _sz__STR(__LINE__)  )
#  undef _sz__STR
#  undef _sz__STR_2
# endif // _MSC_VER
#
# undef WARNING
#
#endif

#ifdef NOTE
# ifndef _MSC_VER
#  warning "Oh no, someone beat us to it: NOTE is already taken! ;) But never fear, let's just #undef it like a boss..."
# else
#  define _sz__STR_2(x) #x
#  define _sz__STR(x) _sz__STR_2(x)
#  pragma message( "Warning: Oh no, someone beat us to it: NOTE is already taken! ;) But never fear, let's just #undef it like a boss..." \
	" at " __FILE__ ":" _sz__STR(__LINE__)  )
#  undef _sz__STR
#  undef _sz__STR_2
# endif // _MSC_VER
#
# undef NOTE
#
#endif

#ifdef BUG
# ifndef _MSC_VER
#  warning "Oh no, someone beat us to it: BUG is already taken! ;) But never fear, let's just #undef it like a boss..."
# else
#  define _sz__STR_2(x) #x
#  define _sz__STR(x) _sz__STR_2(x)
#  pragma message( "Warning: Oh no, someone beat us to it: BUG is already taken! ;) But never fear, let's just #undef it like a boss..." \
	" at " __FILE__ ":" _sz__STR(__LINE__)  )
#  undef _sz__STR
#  undef _sz__STR_2
# endif // _MSC_VER
#
# undef BUG
#
#endif

#ifdef ABORT
# ifndef _MSC_VER
#  warning "Oh no, someone beat us to it: ABORT is already taken! ;) But never fear, let's just #undef it like a boss..."
# else
#  define _sz__STR_2(x) #x
#  define _sz__STR(x) _sz__STR_2(x)
#  pragma message( "Warning: Oh no, someone beat us to it: ABORT is already taken! ;) But never fear, let's just #undef it like a boss..." \
	" at " __FILE__ ":" _sz__STR(__LINE__)  )
#  undef _sz__STR
#  undef _sz__STR_2
# endif // _MSC_VER
#
# undef ABORT
#
#endif

//----------------------------------------------------------------------------
// API...
//----------------------------------------------------------------------------

#include <string> // for ""s
using namespace std::string_literals;
//! <sstream> would be nice for << support, but that's almost <iostream>-level heavy.
//! Maybe integrating plog's nostream instead? (This is a sibling of sz/log anyway!)

// Note:
//
//  - The `""s + ...` hack is to support concatenating const char* literals directly
//    in calls like `NOTE("Found " + filename)` (instead of `"Found "s + ...`,
//    which is easy to forget, and ugly too).
//
//!! The vararg. syntax is not implemented yet! (Mostly due to the legacy
//!! preprocessor still being the default in MSVC, complicating things.)
//!!
//!! And the evern more flexible fmt syntax is not supported because of the heavy
//!! dependency of std::format. (Error.hpp is assumed to be included for many TUs,
//!! so any significant compilation burden here could be painful.) The current
//!! general policy is _removing_ std C++ dependencies, not adding new ones...
//
//!! Get rid of the Szim:: namespace?! OK, but then move to sz::? Just removing the ns. qual. here feels reckless.
#define NOTE(msg, ...)        ( Szim::NOTE_impl    ({__FUNCTION__, __LINE__}, ""s + msg) )
#define WARNING(msg, ...)     ( Szim::WARNING_impl ({__FUNCTION__, __LINE__}, ""s + msg) )
#define ERROR(msg, ...)       ( Szim::ERROR_impl   ({__FUNCTION__, __LINE__}, ""s + msg) )
#define FATAL_ERROR(msg, ...) ( Szim::FATAL_impl   ({__FUNCTION__, __LINE__}, ""s + msg) )
#define BUG(msg, ...)         ( Szim::BUG_impl     ({__FUNCTION__, __LINE__}, ""s + msg) )

//!! Another subtle loss for #623 is not being able to apply the nice concat. support hack of `""s + msg`:
//void bug(string_view msg, std::_source_location loc = std::_source_location::current());

#define ABORT(...)            ( Szim::ABORT_impl   ({__FUNCTION__, __LINE__} __VA_OPT__(,) __VA_ARGS__) ) //! Requires -Zc::preprocessor for MSVC

//----------------------------------------------------------------------------
// Impl...
//----------------------------------------------------------------------------

#include <string_view>

// Only guard this part of the heder!
#ifndef _CM029874Y687G43YT078GN6RTCF039VM74B5NTYN798_
#define _CM029874Y687G43YT078GN6RTCF039VM74B5NTYN798_


namespace Szim {

struct SourceLocation {
	std::string_view fn;
	int ln = 0; // 0 indicates a "NULL" location, which should not be printed etc...
	explicit operator bool () const { return ln; }
};

struct FatalError {
	std::string_view message;
	SourceLocation loc;
};

void NOTE_impl    (const SourceLocation& loc, std::string_view message = "", ...);
void WARNING_impl (const SourceLocation& loc, std::string_view message = "", ...);
void ERROR_impl   (const SourceLocation& loc, std::string_view message = "", ...);
void FATAL_impl   (const SourceLocation& loc, std::string_view message = "", ...);
void BUG_impl     (const SourceLocation& loc, std::string_view message = "", ...);

void ABORT_impl   (const SourceLocation& loc = {}, std::string_view message = "", ...);

} // namespace Szim


#endif // #define _CM029874Y687G43YT078GN6RTCF039VM74B5NTYN798_