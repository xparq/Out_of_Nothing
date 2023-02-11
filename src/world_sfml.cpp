#include "world_sfml.hpp"
#include "engine_sfml.hpp"

#include <cmath> // pow?, sqrt
#include <iostream> // cerr
#include <cassert>

using namespace std;

//----------------------------------------------------------------------------
size_t World::add_body(Body&& obj)
{
	obj.precalc();
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
void World_SFML::recalc_for_next_frame(Engine_SFML& engine)
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	if (paused()) return;

	dt = clock.getElapsedTime().asSeconds();
	clock.restart();

	// Go through the autogen effects first:
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Thrust -- for objects with working thrusters...:
		if (body->has_thrusters()) {
			sf::Vector2f F_thr( (-body->thrust_left.thrust_level() + body->thrust_right.thrust_level()) * dt,
							    (-body->thrust_up.thrust_level() + body->thrust_down.thrust_level()) * dt);
			body->v += (F_thr / body->mass);
		}
	}

// Now do the interaction matrix:
//!!This line below is hard-coded to globe-ndx == 0, and also ignores any other (potential) players!...
for (size_t actor_obj_ndx = 0; actor_obj_ndx < (_interact_all ? bodies.size() : 1); ++actor_obj_ndx)

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Gravity
		if (!body->superpower.gravity_immunity // <- Ignore gravity on player superglobes by default!
			&& i != actor_obj_ndx) {
//		if (i != actor_obj_ndx) {
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
				static constexpr float EPS_COLLISION = engine.CFG_GLOBE_RADIUS/10; //!! experimental guesstimate (was: 100000); should depend on the relative speed!
				if (abs(distance - (body->r + other->r)) < EPS_COLLISION ) {
//cerr << "Touch!\n";
					if (!engine.touch_hook(this, body.get(), other.get())) {
						;
					}
				} else {
//cerr << " - Collided, but NO TOUCH. d = " << distance << ", delta = "<<abs(distance - (body->r + other->r)) << " (epsilon = "<<EPS_COLLISION<<")\n";
				}

				// Note: calling the hook before processing the collision!
				// If the listener returns false, it didn't process it, so we should.
				if (!engine.collide_hook(this, body.get(), other.get(), distance)) {
					//!!Possibly also handle this in a hook, but one of the physics.

					//!! Interestingly, if this spee reset below is disabled, an orbiting moon
					//!! that hits the surface of the Superglobe (in a flat-enough angle),
					//!! DOES bounce off of it! :-o WHY?
					//body->v = {0, 0}; // or bounce, or stick to the other body and take its v, or any other sort of interaction.
				}

				//! Also call a high-level, "predefined emergent interaction" hook:
				engine.interaction_hook(this, Event::Collision, body.get(), other.get());
			} else { // i.e. not colliding:
				float g = G * other->mass / (distance * distance);
				sf::Vector2f gvect(dx * g, dy * g);
				//!!should rather be: sf::Vector2f gvect(dx / distance * g, dy / distance * g);
				sf::Vector2f dv = gvect * dt;
				body->v += dv;
//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<body->v.x<<","<<body->v.y<<") " << endl;
			}
		}
/*!! Very interesting magnified effect if calculated here, esp. with negative friction -- i.e. an expanding universe:
		// Friction:
		sf::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
		sf::Vector2f dv = friction_decel * (dt);
		body->v += dv;
!!*/		
//cerr << "v["<<i<<"] = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
	}

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Friction:
		sf::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
		sf::Vector2f dv = friction_decel * (dt);
		body->v += dv;
		
		// And finally the positions:
		sf::Vector2f ds(body->v.x * dt, body->v.y * dt);
		body->p += ds;
	}
}
