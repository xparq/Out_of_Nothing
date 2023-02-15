#ifndef __MODEL__MATHS__
#define __MODEL__MATHS__

#include <cmath> // sqrt

namespace Model { //!!Should probably escape to an even more generic space!

constexpr float MyNaN = 2e31f; // cringy way to avoid the pain of using the std NAN ;)

//!!Put these into some generic geometry helper thing:
	//!! There might also be a need for a "fast" version that's only used to
	//!! sort/select/differentiate objects by distance, so the sqrt can be skipped!
template <typename T> T distance_2d(T dx, T dy)  { return sqrt(dx*dx + dy*dy); }
template <typename T> T distance_2d(T x1, T y1, T x2, T y2)
{
	T dx = x2 - x1, dy = y2 - y1;
	return distance_2d(dx*dx, dy*dy);
}

}; // namespace
#endif // __MODEL__MATHS__
