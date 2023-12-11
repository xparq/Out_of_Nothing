#ifndef _Y8GVVY7TC880X820KS272475BTBT7V_
#define _Y8GVVY7TC880X820KS272475BTBT7V_

#include "Math.hpp"

#include <cstdint> // uint32_t

namespace Model {

struct Physics
{
	static /*constexpr*/ float T_to_BV(float T); //!! just faking something simple
	// "Hot stars have temperatures around 60,000 K while cold stars have temperatures around 3,000 K"
	// But the hottest is around 200000 K.
	// So... Let's just stick to a 0 - 100000 K range

	// Color temp. conversion from B-V val.:
	// https://stackoverflow.com/a/22630970/1479945:
	static /*constexpr*/ float BV_to_T_and_RGB(float bv, uint32_t* p_color = nullptr); // BV <-0.4,+2.0> -> RGB <0,1>
	//! The result must be shitfed <<8 for SFML's sf::Color!
	//! E.g. that's exactly what the OON renderer does, so it's
	//! fine to just store this directly in the objects.

	constexpr     static float mass_from_radius_and_density(float r, float d)
	                               { return Math::FOUR_THIRD_PI*r*r*r * d; } //!!?? powf() is not constepxr?! WTF??}
	/*constexpr*/ static float radius_from_mass_and_density(float m, float d)
	                               { return std::powf(m/d/Math::FOUR_THIRD_PI, 1/3.f); } //!!?? powf() is not constepxr?! WTF??}

	// OK, but now just using this quick-and-dirty impromptu hack, instead of all the above... ;)
	static float T_to_RGB_and_BV(float T, uint32_t* p_color = nullptr);

	//! Wow, C++ `const` can't do non-integer static?! :-o
	static constexpr float G = 6.673e-11f; //!! No point keeping this real and all the others stretched,
	                                       //!! unless a real orbital simulation is the goal (which isn't)!...

	static constexpr float RADIUS_OF_EARTH = 6371000.f; // km
	static constexpr float MASS_OF_EARTH = 6e24f; // kg
	static constexpr float DENSITY_OF_EARTH = 5500.0f; // kg/m3
	static constexpr float DENSITY_ROCK = 2000.0f; // kg/m3
};

}; // namespace Model

#endif // _Y8GVVY7TC880X820KS272475BTBT7V_
