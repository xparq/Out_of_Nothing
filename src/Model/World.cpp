#include "Model/World.hpp"
#include "SimApp.hpp"

#include <cmath> // pow?, sqrt
#include <iostream> // cerr
#include <cassert>

using namespace std;

namespace Model {


//============================================================================
float World::Physics::T_to_BV(float T) //!! just faking something simple
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
	constexpr float T_BV_MIN = 3000;
	constexpr float T_BV_MAX = 200000;
//cerr << "T->BV: T = " << T << ", BV = " << -0.4f + 2.4f * T / T_MAX << "\n";
	return T < T_BV_MIN || T > T_BV_MAX
			? MyNaN
			: -0.4f + 2.4f * // normal BV range is 2.4
				T / T_BV_MAX;
}

// Color temp. conversion from B-V val.:
// https://stackoverflow.com/a/22630970/1479945:
float World::Physics::BV_to_T_and_RGB(float bv, uint32_t* p_color/* = nullptr*/) // BV: [-0.4,+2.0]
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
	else if ((bv>= 0.00f)&&(bv<0.40f)) { t=(bv-0.00f)/(0.40f-0.00f); r=0.83f+(0.17f*t)          ; }
			if ((bv>=-0.40f)&&(bv<0.00f)) { t=(bv+0.40f)/(0.00f+0.40f); r=0.61f+(0.11f*t)+(0.1f*t*t); }
	else if ((bv>= 0.40f)&&(bv<2.10f)) { t=(bv-0.40f)/(2.10f-0.40f); r=1.00f                    ; }
			if ((bv>=-0.40f)&&(bv<0.00f)) { t=(bv+0.40f)/(0.00f+0.40f); g=0.70f+(0.07f*t)+(0.1f*t*t); }
	else if ((bv>= 0.00f)&&(bv<0.40f)) { t=(bv-0.00f)/(0.40f-0.00f); g=0.87f+(0.11f*t)          ; }
	else if ((bv>= 0.40f)&&(bv<1.60f)) { t=(bv-0.40f)/(1.60f-0.40f); g=0.98f-(0.16f*t)          ; }
	else if ((bv>= 1.60f)&&(bv<2.00f)) { t=(bv-1.60f)/(2.00f-1.60f); g=0.82f          -(0.5f*t*t); }
			if ((bv>=-0.40f)&&(bv<0.40f)) { t=(bv+0.40f)/(0.40f+0.40f); b=1.00f                    ; }
	else if ((bv>= 0.40f)&&(bv<1.50f)) { t=(bv-0.40f)/(1.50f-0.40f); b=1.00f-(0.47f*t)+(0.1f*t*t); }
	else if ((bv>= 1.50f)&&(bv<1.94f)) { t=(bv-1.50f)/(1.94f-1.50f); b=0.63f          -(0.6f*t*t); }
	if (p_color) {
		*p_color = (uint8_t(r*255) << 16) | (uint8_t(g*255) << 8) | uint8_t(b*255);
//cerr << "calculated new color from BV " << bv << ": " << hex << *p_color << " r:"<<r<<" g:"<<g<<" b:"<<b<< "\n";
	}
	return t;
}


//============================================================================
void World::Body::recalc()
{
	mass = powf(r, 3) * density;
	Physics::BV_to_T_and_RGB(Physics::T_to_BV(T), superpower.free_color ? nullptr: &color);
}




//============================================================================
size_t World::add_body(Body&& obj)
{
	obj.recalc();
	bodies.push_back(std::make_shared<Body>(obj));

	return bodies.size() - 1;
}

void World::remove_body(size_t ndx)
{
	assert(bodies.size() > 0);
	assert(ndx != (size_t)-1);
	bodies.erase(bodies.begin() + ndx);
}

//----------------------------------------------------------------------------
void World::recalc_next_state(float dt, SimApp& game)
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
#ifdef _SKIP_
//!!testin' testin'...
static int SKIP_TIMES = 0; //! const[expr] here would trigger a warning later for if (0)
static int skipping_interactions = SKIP_TIMES;
static float accumulated_dt = 0; // s
auto last_dt = dt;
#endif

	// Go through the autogenic effects first:
	//!!This should be a separate collection tho; super wasteful to go thru all every time!
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Thrust -- for objects with working thrusters...:
		if (body->has_thrusters()) {
			sfml::Vector2f F_thr( (-body->thrust_left.thrust_level() + body->thrust_right.thrust_level()) * dt,
							    (-body->thrust_up.thrust_level() + body->thrust_down.thrust_level()) * dt);
			body->v += (F_thr / body->mass);
		}
	}

#ifdef _SKIP_
//!!testin' testin'...
if (SKIP_TIMES && --skipping_interactions) {
	accumulated_dt += dt; goto end_interact_loop;
} else {
//	cerr << ".";
	if (accumulated_dt > 0) { dt = accumulated_dt; accumulated_dt = 0; }
	skipping_interactions = SKIP_TIMES;
}
#endif

// Now do the interaction matrix:
//!!This line below is hard-coded to globe-ndx == 0, and also ignores any other (potential) players!...
for (size_t actor_obj_ndx = 0; actor_obj_ndx < (_interact_all ? bodies.size() : 1); ++actor_obj_ndx)

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Collisions & gravity...
		if (i != actor_obj_ndx) {
			auto& other = bodies[actor_obj_ndx];

			auto dx = other->p.x - body->p.x,
			     dy = other->p.y - body->p.y;

//			auto distance = distance_2d(body->p.x, body->p.y, other->p.x, other->p.y);
			auto distance = distance_2d(dx, dy);

			//! Collision det. is crucial also for preventing 0 distance to divide by!
			//!!Collision and "distance = 0" are not the samne things!
			//!!Collision concerns the surfaces, but "distance" here for gravity
			//!!is measured from the center (of mass) of the body!
			//!!But d = 0 is a "subset" of collisions, so if there would be a "safe"
			//!!adjustment for any collision to guarantee distance != 0, that would be
			//!!enough...

			//!!Also, obviously, "almost 0" could be just as bad as 0 (which is not
			//!!even quite real in float, and would anyway just yield a NaN), so
			//!!the real solution would be smarter modeling of the high-speed case
			//!!to do what nature does... Dunno, create a black hole, if you must! :)

			if (/*physics.*/is_colliding(body.get(), other.get(), distance)) {
			//! Done before actually reaching the body (so we can avoid the occasional frame showing the penetration :) )!
			//  (Opting for the perhaps even less natural "but didn't even touch!" issue...)

				// If the collision results in a well-defined fixed position/velocity etc.,
				// they may need special treatment, because at this point the checked body may not
				// yet have reached (or crossed the boundary of) the other, so it needs to be adjusted
				// to the expected collision end-state!
				//!!/*physics.*/collide_hook(body.get(), other.get(), distance);
				//! Call God, too:

				// Call this before processing the collision-induced speed changes,
				// so that the calc. can take into consideration the relative speed!
//!!			auto rel_dv = distance_2d(body->v.x, body->v.y, other->v.x, other->v.y);
				static constexpr float EPS_COLLISION = CFG_GLOBE_RADIUS/10; //!! experimental guesstimate (was: 100000); should depend on the relative speed!
				if (abs(distance - (body->r + other->r)) < EPS_COLLISION ) {
//cerr << "Touch!\n";
					if (!game.touch_hook(this, body.get(), other.get())) {
						;
					}
				} else {
//cerr << " - Collided, but NO TOUCH. d = " << distance << ", delta = "<<abs(distance - (body->r + other->r)) << " (epsilon = "<<EPS_COLLISION<<")\n";
				}

				// Note: calling the hook before processing the collision!
				// If the listener returns false, it didn't process it, so we should.
				if (!game.collide_hook(this, body.get(), other.get(), distance)) {
					//!!Possibly also handle this in a hook, but one of the physics.

					//!! Interestingly, if this spee reset below is disabled, an orbiting moon
					//!! that hits the surface of the Superglobe (in a flat-enough angle),
					//!! DOES bounce off of it! :-o WHY?
					//body->v = {0, 0}; // or bounce, or stick to the other body and take its v, or any other sort of interaction.
				}

				//! Also call a high-level, "predefined emergent interaction" hook:
				game.interaction_hook(this, Event::Collision, body.get(), other.get());

			} else if (!body->superpower.gravity_immunity) { // process gravity if not colliding
				float g = G * other->mass / (distance * distance);
				sfml::Vector2f gvect(dx * g, dy * g);
				//!!should rather be: sfml::Vector2f gvect(dx / distance * g, dy / distance * g);
				sfml::Vector2f dv = gvect * dt;
				body->v += dv;
//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<body->v.x<<","<<body->v.y<<") " << endl;
			}
		}
/*!! Very interesting magnified effect if calculated here, esp. with negative friction -- i.e. an expanding universe:
		// Friction:
		sfml::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
		sfml::Vector2f dv = friction_decel * (dt);
		body->v += dv;
!!*/		
//cerr << "v["<<i<<"] = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
	}

#ifdef _SKIP_
end_interact_loop:
dt = last_dt;
#endif

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Friction:
		sfml::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
		sfml::Vector2f dv = friction_decel * (dt);
		body->v += dv;
		
		// And finally the positions:
		sfml::Vector2f ds(body->v.x * dt, body->v.y * dt);
		body->p += ds;
	}
}

} // namespace