#include "Physics.hpp"

namespace Model {

using namespace Math;

static constexpr float T_BV_MIN = 15000;
static constexpr float T_BV_MAX = 200000;

float Physics::T_to_BV(float T) //!! just faking something simple
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
//	constexpr float T_BV_MIN = 3000;
//	constexpr float T_BV_MAX = 200000;
//cerr << "T->BV: T = " << T << ", BV = " << -0.4f + 2.4f * T / T_MAX << "\n";
	return T < T_BV_MIN || T > T_BV_MAX
			? MyNaN
			: -0.4f + 2.4f * // normal BV range is 2.4
				T / T_BV_MAX;
}

// Temp. -> color conversion
float Physics::T_to_RGB_and_BV(float T, uint32_t* p_color/* = nullptr*/) // BV: [-0.4,+2.0]
//! The resulting color must be shitfed <<8 to match SFML's sf::Color.
//! E.g. that's exactly what the OON renderer does, so it's fine to just
//! store this directly in the objects.
{
	float r = 0.f, g = 0.f, b = 0.f;
	float bv = MyNaN;
	if (T < T_BV_MIN)
		return MyNaN;
	if (T > T_BV_MAX) {
		r = b = g = 0.0f; // #138: "black holes" ;)
	} else {
		bv = -0.4f + 2.4f * // normal BV range is 2.4
			            T / T_BV_MAX;
		float t;
		if (bv < -0.4f) bv = -0.4f; if ( bv > 2.0f) bv = 2.0f;
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


#if 0
// Color temp. conversion from B-V val.:
// https://stackoverflow.com/a/22630970/1479945:
float Physics::BV_to_T_and_RGB(float bv, uint32_t* p_color/* = nullptr*/) // BV: [-0.4,+2.0]
//! The resulting color must be shitfed <<8 to match SFML's sf::Color.
//! E.g. that's exactly what the OON renderer does, so it's fine
//! to just store this directly in the objects.
//!
//!! The calculated T value is not used currently, but kept for future ref.
//!! (A BV->T function might come in handy later!)
{
	float t = 0.f;
	float r = 0.f, g = 0.f, b = 0.f;
	if (bv == MyNaN) //! See T_to_BV()...
		return MyNaN;
	if (bv < -0.4f) bv = -0.4f; if ( bv > 2.0f) bv = 2.0f;
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
	if (p_color) {
		*p_color = (uint8_t(r*255) << 16) | (uint8_t(g*255) << 8) | uint8_t(b*255);
//cerr << "calculated new color from BV " << bv << ": " << hex << *p_color << " r:"<<r<<" g:"<<g<<" b:"<<b<< "\n";
	}
	return t;
}
#endif

}; // namespace