#include "Model/World.hpp"
#include "SimApp.hpp"

#include <cmath> // sqrt, pow?
#include <iostream> // cerr
#include <cassert>

using namespace std;

namespace Model {

size_t World::add_body(Body const& obj)
{
	bodies.push_back(std::make_shared<Body>(obj));
	auto ndx = bodies.size() - 1;
	bodies[ndx]->recalc();
	return ndx;
}

size_t World::add_body(Body&& obj)
{
	obj.recalc(); // just recalc the original throw-away obj
	bodies.emplace_back(std::make_shared<Body>(obj));
	return bodies.size() - 1;
}

void World::remove_body(size_t ndx)
{
	assert(bodies.size() > 0);
	assert(ndx != (size_t)-1);
	bodies.erase(bodies.begin() + ndx);
}

#define _SKIP_
//#define _SKIP_INTERACTIONS_
//----------------------------------------------------------------------------
void World::recalc_next_state(float dt, SimApp& game)
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
//	static float t = 0; // s
//
//	t += dt;

#ifdef _SKIP_
//!!testin' testin'...
static int SKIP_N = 10; //! const[expr] here would trigger a warning for a later `if (0)`
static int skipping_T_recalc = SKIP_N * 10;
static int skipping_n_interactions = SKIP_N;
static float accumulated_skip_dt = 0; // s
auto last_dt = dt;
#endif

	// Go through autogenic effects first:
	//!!This should be a separate collection tho; super wasteful to go thru all every time!
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Lifecycle mgmt... (kill decaying objects before wasting effort on them)
		if (body->can_decay()) {
			body->lifetime -= dt;
			if (body->lifetime <= 0) {
				body->on_event(Event::Decay); //! NOTE: it's not the obj. that does the decaying it's just notified!

				//!!bodies.erase(i);... //!!Watch out for the loop then, as this would make the indexes/iterators invalid!!
										//!!Better just to mark it DELETED, and let another loop remove them later!
//cerr << "#"<<i <<" DECAYED\n";
				continue;
			}
		}

		// As a placeholder for real thermodynamics, just auto-cool objects for now:
		if (body->T > 0) {
			body->T *= 0.996f; //!! even 0.995 already cools back down too fast to form "black holes"

			//!! Here should be an optimized progression to different "T color regimes"
			//!! to avoid calling recalc() on every single tick!
			//!! The temp. change should happen, and then either a quick condition here
			//!! should detect change in "abstract color" (similar to B-V), or the renderer
			//!! should always unconditionally map T to real color...
			//!!
			//!! Wel, OK, doing a "skiprate" refresh here, so that's gonna be the condition then...
#ifdef _SKIP_
			if (!--skipping_T_recalc) {
				skipping_T_recalc = SKIP_N * 10;
				body->recalc();
//cerr << "#"<<i <<" Recalculated (T = " << body->T << ").\n";
			}
#endif
//cerr << "#"<<i <<" Cooled to " << body->T << "...\n";
		}

		// Thrust -- for objects with working thrusters...:
		if (body->has_thrusters()) {
			sfml::Vector2f F_thr( (-body->thrust_left.thrust_level() + body->thrust_right.thrust_level()) * dt,
							    (-body->thrust_up.thrust_level() + body->thrust_down.thrust_level()) * dt);
			body->v += (F_thr / body->mass);
		}
	}

#if defined(_SKIP_) && defined(_SKIP_INTERACTIONS_)
//!!testin' testin'...
if (SKIP_N && --skipping_n_interactions) {
	accumulated_skip_dt += dt; goto end_interact_loop;
} else {
//	cerr << ".";
	// Consume the time we've "collected" while skipping:
	if (accumulated_skip_dt > 0) { dt = accumulated_skip_dt; accumulated_skip_dt = 0; }
	// Start skipping again:
	skipping_n_interactions = SKIP_N;
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
				float g = Physics::G * other->mass / (distance * distance);
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

#ifdef _SKIP_INTERACTIONS_
end_interact_loop:
dt = last_dt; // Restore "real dt" for calculations outside the "skip cheat"!
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

//----------------------------------------------------------------------------
World& World::_clone(World const& other)
{
	//!!Move these into some container to avoid forgetting
	//!!some when manip. them one by one! :-/
	//!!There might anyway be a distinction between these and
	//!!throw-away volatile state (like caches) in the future.
	FRICTION = other.FRICTION;
	_interact_all = other._interact_all;

	bodies.clear();
	for (auto const& b : other.bodies) {
		add_body(*b);
	}
//cerr << "World cloned.\n";
	return *this;
}

} // namespace Model