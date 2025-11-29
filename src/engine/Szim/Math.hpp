#ifndef _C4875687UTYVCM056V89YN0T7NHPB8OIEIUCYM4M40576_
#define _C4875687UTYVCM056V89YN0T7NHPB8OIEIUCYM4M40576_

/*
	Math features here that are only used by the Model.
	As soon as something becomes useful for -- and used by -- any other
	part of the system, it should be "exported" out (to e.g. sz::).

	But basically this still tries to remain an embedded, internal feature
	of the Model, rather than an external depdendency. (As a matter of fact,
	it could go even deeper, embedded into the -- replaceable -- physics,
	if that's the only part actually using this!!)
*/

//! Client code should #include Math/* on its own, as needed!
//#include "Math/Vector2.hpp"
//#include "Math/Vector3.hpp"
	//! NOTE: these have been ripped out from SFML, so I can keep using them,
	//! while also having been nicely decoupled from SFML here.
	//! To avoid duplicate definitions, the local version has changed the
	//! `sf` namespace to `Math`.
	//! Alas, that also means the actual types, albeit isomorphic, are now
	//! syntactically incompatible, requiring awkward explicit conversions
	//! (with copies, for now! :-/ ), via e.g. "adapter/SFML/vector.hpp"!
	//!! SHOULD BE MADE IMPLICIT & COMPILE-TIME-ONLY!


#include <cmath> // pow, sqrt

namespace Math {

//!! Perhaps put the alternatives into a `detail` ns, and then "canonicalize"
//!! only the one selected (by a templ. arg.)?
//!! (Or would there be a use case where multiple different num. types would
//!! need to be available at the same time?!)

template <typename T> constexpr T MyNaN = T(2e31f); //!! Cringy way to avoid the pain of using the std NAN ;) -> #527
template <typename T> constexpr T PI = T(3.1415926535897932385); //!! Should be templated, too!
template <typename T> constexpr T FOUR_THIRD_PI = T(PI<T> * T(4)/T(3)); //!!4.1887902f;
	template <> inline constexpr auto FOUR_THIRD_PI<float> = float(PI<float> * 4.f/3.f);
	template <> inline constexpr auto FOUR_THIRD_PI<double> = double(PI<double> * 4.0/3.0);
	template <> inline constexpr auto FOUR_THIRD_PI<long double> = (long double)(PI<long double> * 4.0L/3.0L); //! GCC can't parse `long double(...)`

template <typename T> inline T power(T base, T exp) {}
	//!! cmath is not constepxr in general (so, no constexpr pow*() either)! :-/
	template <> inline float       power<float>(float base, float exp) { return powf(base, exp); }
	template <> inline double      power<double>(double base, double exp) { return pow(base, exp); }
	template <> inline long double power<long double>(long double base, long double exp) { return powl(base, exp); }


//!! There might also be a need for a "fast" version (like mag2_sq) that's only used
//!! to sort/select/differentiate objects by distance, so the sqrt can be skipped!
template <typename T> T mag2(T dx, T dy)  { return sqrt(dx*dx + dy*dy); }
template <typename T> T distance2(T x1, T y1, T x2, T y2) { return mag2(x2 - x1, y2 - y1); }

//!! Don't do this, this is not a Math lib! Only add stuff here that's *actually*
//!! needed/used!...
//!! (And definitely use vectors for this one, BTW... :)z2 - z1
template <typename T> T mag3(T dx, T dy, T dz)  { return sqrt(dx*dx + dy*dy + dz*dz); }
template <typename T> T distance3(T x1, T y1, T z1, T x2, T y2, T z2) { return mag3(x2 - x1, y2 - y1, z2 - z1); }

} // namespace Math

#endif // _C4875687UTYVCM056V89YN0T7NHPB8OIEIUCYM4M40576_
