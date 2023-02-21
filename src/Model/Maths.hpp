#ifndef __MODEL__MATHS__
#define __MODEL__MATHS__

/*
	Math features here that are only used by the Model.
	As soon as something becomes useful for -- and used by -- any other part
	of the system, it should be "exported" out (to e.g. "misc" instead).

	IOW, this tries to remain an embedded, internal feature of the Model,
	rather than an external depdendency. (As a matter of fact, it could go
	even deeper, embedded into the -- replaceable -- physics, if that's the
	only part actually using this!!)
*/

#include <cmath> // sqrt

namespace Model { //!!Should probably escape to an even more generic space!

constexpr float MyNaN = 2e31f; // cringy way to avoid the pain of using the std NAN ;)

//!!Put these into some generic geometry helper thing:
	//!! There might also be a need for a "fast" version that's only used to
	//!! sort/select/differentiate objects by distance, so the sqrt can be skipped!
template <typename T> T distance_2d(T dx, T dy)  { return sqrt(dx*dx + dy*dy); }
template <typename T> T distance_2d(T x1, T y1, T x2, T y2) { return distance_2d(x2 - x1, y2 - y1); }

//!! Don't do this, this is not a Math lib! Only add stuff here that's *actually*
//!! needed/used!...
//!! (And definitely use vectors for this one, BTW... :)z2 - z1
template <typename T> T distance_3d(T dx, T dy, T dz)  { return sqrt(dx*dx + dy*dy + dz*dz); }
template <typename T> T distance_3d(T x1, T y1, T z1, T x2, T y2, T z2) { return distance_3d(x2 - x1, y2 - y1, z2 - z1); }

}; // namespace
#endif // __MODEL__MATHS__
