#ifndef _MODEL__MATH_
#define _MODEL__MATH_

/*
	Math features here that are only used by the Model.
	As soon as something becomes useful for -- and used by -- any other
	part of the system, it should be "exported" out (to e.g. sz::).

	But basically this still tries to remain an embedded, internal feature
	of the Model, rather than an external depdendency. (As a matter of fact,
	it could go even deeper, embedded into the -- replaceable -- physics,
	if that's the only part actually using this!!)
*/

//! Client code should include Math/* on its own, as needed!
//#include "Math/Vector2.hpp"
//#include "Math/Vector3.hpp"
	//! NOTE: these have been ripped out from SFML, so I can keep using them,
	//! while also having been nicely decoupled from SFML here.
	//! To avoid duplicate definitions, the local version has changed the
	//! `sf` namespace to `Math`.
	//! Alas, that also means the actual types, albeit isomorphic, are now
	//! syntactically incompatible, requiring awkward explicit conversions
	//! (with copies!), via e.g. "adapter/SFML/vector.hpp"! :-/
	//!! Those could be made implicit though, at least in some cases, right?!


#include <cmath> // sqrt

namespace Math {

constexpr const float MyNaN = 2e31f; // cringy way to avoid the pain of using the std NAN ;)
constexpr const float FOUR_THIRD_PI = 4.1887902f;

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

#endif // _MODEL__MATH_
