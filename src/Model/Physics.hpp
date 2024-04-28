#ifndef _Y8GVVY7TC880X820KS272475BTBT7V_
#define _Y8GVVY7TC880X820KS272475BTBT7V_

#include "cfg.h" // Model::BasicNumberType


#include "Math.hpp"

#include <cstdint> // uint32_t for colors... --> Physics/Color.hpp!!

namespace Model {

// Make sure the configured basic number type is used by each client of the model physics:
template <typename NumT> class Physics;
using Phys = Physics<Model::BasicNumberType>; //!! The same should be done for Math, too!

template <typename NumT> class Physics
{
public:
	using NumType = NumT;
	//!! Multiple number types should also be supported:
	//!!using DynamicsNumberType = BasicNumberType;
	//!!...

	//--------------------------------------------------------------------
	// Constants...
	//----------------

	//! Wow, C++ `const` can't do non-integer static?! :-o
	static constexpr auto G = NumType(6.67430e-11); //!! No point keeping this real and all the others stretched,
	                                                //!! unless a real orbital simulation is the goal (which isn't)!...
	static constexpr auto RADIUS_OF_EARTH  = NumType(6371000); // km
	static constexpr auto MASS_OF_EARTH    = NumType(6e24); // kg
	static constexpr auto DENSITY_OF_EARTH = NumType(5500); // kg/m3
	static constexpr auto DENSITY_ROCK     = NumType(2000); // kg/m3

	//--------------------------------------------------------------------
	// Formulas...
	//---------------

	static inline /*constexpr*/ NumType T_to_BV(NumType T); //!! just faking something simple
	// "Hot stars have temperatures around 60,000 K while cold stars have temperatures around 3,000 K"
	// But the hottest is around 200000 K.
	// So... Let's just stick to a 0 - 100000 K range

	// Color temp. conversion from B-V val.:
	// https://stackoverflow.com/a/22630970/1479945:
	static inline /*constexpr*/ NumType BV_to_T_and_RGB(NumType bv, uint32_t* p_color = nullptr); // BV <-0.4,+2.0> -> RGB <0,1>
	//! The result must be shitfed <<8 for SFML's sf::Color!
	//! E.g. that's exactly what the OON renderer does, so it's
	//! fine to just store this directly in the objects.

	constexpr     static NumType mass_from_radius_and_density(NumType r, NumType d)
	                               { return NumType(Math::FOUR_THIRD_PI<NumType>)* r*r*r * d; }
	/*constexpr*/ static NumType radius_from_mass_and_density(NumType m, NumType d)
	                               { return NumType(Math::power(m/d/NumType(Math::FOUR_THIRD_PI<NumType>), NumType(1)/NumType(3))); } //!! cmath's pow() is not constepxr! :-o

	// Temp. -> color conversion
	// OK, but now just this quick-and-dirty impromptu hack, instead of all the above... ;)
	static inline NumType T_to_RGB_and_BV(NumType T, uint32_t* p_color = nullptr);

private:
	static constexpr auto T_BV_MIN = NumType(15000);
	static constexpr auto T_BV_MAX = NumType(200000);
};

} // namespace Model


// Inl. impl...
namespace Model {

template <typename NumType> NumType Physics<NumType>::T_to_BV(NumType T) //!! just faking something simple
{
	// "Hot stars have temperatures around 60,000 K while cold stars have temperatures around 3,000 K"
	// But the hottest is around 200000 K, so...
	// Let's just calibrate for a 3000 - 200000 K range.
	// Mmm, even tho we aren't even dealing with stars, BTW!... :)
	// OK, so here's the deal:
	// - out-of-range T would result in a fake BV that
	// - the BV->RGB converter would notice, and not touch the input color!
	// This would allow nice random-color balloons to float around, until getting hot...
	// The fake value will be MyNaN.
//	constexpr NumType T_BV_MIN = 3000;
//	constexpr NumType T_BV_MAX = 200000;
//cerr << "T->BV: T = " << T << ", BV = " << -0.4f + 2.4f * T / T_MAX << "\n";
	return T < T_BV_MIN || T > T_BV_MAX
			? Math::MyNaN<NumType>
			: NumType(-0.4) + NumType(2.4) * // normal BV range is 2.4
				T / T_BV_MAX;
}

template <typename NumType> NumType Physics<NumType>::T_to_RGB_and_BV(NumType T, uint32_t* p_color/* = nullptr*/) // BV: [-0.4,+2.0]
//! The resulting color must be shitfed <<8 to match SFML's sf::Color.
//! E.g. that's exactly what the OON renderer does, so it's fine to just
//! store this directly in the objects.
{
	NumType r = 0, g = 0, b = 0;
	NumType bv = Math::MyNaN<NumType>;
	if (T < T_BV_MIN)
		return Math::MyNaN<NumType>;
	if (T > T_BV_MAX) {
		r = b = g = 0; // #138: "black holes" ;)
	} else {
		bv = NumType(-0.4) + NumType(2.4) * // normal BV range is 2.4
			            T / T_BV_MAX;
		NumType t;
		if (bv < -0.4f) { bv = -0.4f; } if ( bv > 2.0f) { bv = 2.0f; }
		else if ((bv>= 0.00f)&&(bv<0.40f)) { t=(bv-0.00f)/(0.40f-0.00f); r=0.83f+(0.17f*t)           ; }
			 if ((bv>=-0.40f)&&(bv<0.00f)) { t=(bv+0.40f)/(0.00f+0.40f); r=0.61f+(0.11f*t)+(0.1f*t*t); }
		else if ((bv>= 0.40f)&&(bv<2.10f)) { t=(bv-0.40f)/(2.10f-0.40f); r=1.00f                     ; }
			 if ((bv>=-0.40f)&&(bv<0.00f)) { t=(bv+0.40f)/(0.00f+0.40f); g=0.70f+(0.07f*t)+(0.1f*t*t); }
		else if ((bv>= 0.00f)&&(bv<0.40f)) { t=(bv-0.00f)/(0.40f-0.00f); g=0.87f+(0.11f*t)           ; }
		else if ((bv>= 0.40f)&&(bv<1.60f)) { t=(bv-0.40f)/(1.60f-0.40f); g=0.98f-(0.16f*t)           ; }
		else if ((bv>= 1.60f)&&(bv<2.00f)) { t=(bv-1.60f)/(2.00f-1.60f); g=0.82f          -(0.5f*t*t); }
			 if ((bv>=-0.40f)&&(bv<0.40f)) { t=(bv+0.40f)/(0.40f+0.40f); b=1.00f                     ; }
		else if ((bv>= 0.40f)&&(bv<1.50f)) { t=(bv-0.40f)/(1.50f-0.40f); b=1.00f-(0.47f*t)+(0.1f*t*t); }
		else if ((bv>= 1.50f)&&(bv<1.94f)) { t=(bv-1.50f)/(1.94f-1.50f); b=0.63f          -(0.6f*t*t); }
	}
	if (p_color) {
		*p_color = (uint8_t(r*255) << 16) | (uint8_t(g*255) << 8) | uint8_t(b*255);
//cerr << "calculated new color from BV " << bv << ": " << hex << *p_color << " r:"<<r<<" g:"<<g<<" b:"<<b<< "\n";
	}
	return bv;
}


} // namespace Model

#endif // _Y8GVVY7TC880X820KS272475BTBT7V_
