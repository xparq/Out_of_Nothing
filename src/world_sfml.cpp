#include "world_sfml.hpp"
#include "engine_sfml.hpp"

#include <cmath>
#include <iostream> // cerr
using namespace std;


void World_SFML::recalc_for_next_frame(Engine_SFML& engine)
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	if (paused()) return;

	dt = clock.getElapsedTime().asSeconds();
	clock.restart();

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Thrust:
		if (i == engine.globe_ndx) {
			sf::Vector2f F_thr( (-engine.thrust_left.throttle() + engine.thrust_right.throttle()) * dt,
							    (-engine.thrust_up.throttle() + engine.thrust_down.throttle()) *dt);
			body->v += (F_thr / body->mass);
		}

		// Gravity - only apply to the moon(s), ignore the moon's effect on the globe!
		if (i > engine.globe_ndx) {
			auto& globe = bodies[engine.globe_ndx];
			//float distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));
			float dx = globe->p.x - body->p.x,
			      dy = globe->p.y - body->p.y;
			float distance = sqrt(dx*dx + dy*dy);

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

			if (/*physics.*/is_colliding(body.get(), globe.get(), distance)) {
			//! Done before actually reaching the body (so we can avoid the occasional frame showing the penetration :) )!
			//  (Opting for the perhaps even less natural "but didn't even touch!" issue...)

				// If the collision results in a well-defined fixed position/velocity etc.,
				// they may need special treatment, because at this point the checked body may not
				// yet have reached (or crossed the boundary of) the other, so it needs to be adjusted
				// to the expected collision end-state!
				//!!/*physics.*/collide_hook(body.get(), globe.get(), distance);
				//! Call God, too:
				engine.collide_hook(this, body.get(), globe.get(), distance);

				auto EPS_COLLISION = 100000; //!! experimental guesstimate; should depend on relative speed acautlly!
				if (abs(distance - (body->r + globe->r)) < EPS_COLLISION ) {
					engine.touch_hook(this, body.get(), globe.get());
				}

				//! Also call a high-level, "predefined emergent interaction" hook:
				engine.interaction_hook(this, Event::Collision, body.get(), globe.get());
			} else {
 				float g = G * globe->mass / (distance * distance);
				sf::Vector2f gvect(dx * g, dy * g);
				//!!should rather be: sf::Vector2f gvect(dx / distance * g, dy / distance * g);
				sf::Vector2f dv = gvect * dt;
				body->v += dv;
//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<body->v.x<<","<<body->v.y<<") " << endl;
			}
		}

		// Friction:
		sf::Vector2f friction_decel(-body->v.x * FRICTION, -body->v.y * FRICTION);
		sf::Vector2f dv = friction_decel * (dt);
		body->v += dv;
		sf::Vector2f ds(body->v.x * dt, body->v.y * dt);

		body->p += ds;
//cerr << "v["<<i<<"] = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
	}
}
