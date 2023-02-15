#ifndef __MODEL__PHYSICS__
#define __MODEL__PHYSICS__

#include "Maths.hpp"

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

	//! `const` can't do non-integer statics! :-/
	static constexpr float G = 6.673e-11f; //!! No point keeping this real and all the others stretched,
	               //!! const unless a real orbital simulation is the goal (which isn't)!...
	static constexpr float DENSITY_ROCK = 2000.0f; // kg/m3
};

}; // namespace

#endif // __MODEL__PHYSICS__
