#include "Physics.hpp"
/*!!??
	In case this file gets some real use again:

	How to make sure the templatized Phys. stuff always consistently gets the same
	configured type arg. for each unit that uses it? Others can e.g. just include
	Model.hpp as a pivotal config. point (i.e. aliasing the Phys template for all
	its clients), but not this one!...

	But maybe it shouldn't! After all, templates should live in headers anyway,
	so here only non-generic (non-templated) interanl code should exist!

	Or, just fix the configured number type right in Physics.hpp!

??!!*/


namespace OON::Model {

#if 0
using namespace Math;

static constexpr float T_BV_MIN = 15000;
static constexpr float T_BV_MAX = 200000;

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
#endif

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

}; // namespace OON::Model