#include "Model/World.hpp"

//!! Shrink even this dependency: don't include the entire fucking SimApp,
//!! only its modelling-related services that are actually used:
#include "Engine/SimApp.hpp"

#include "extern/iprof/iprof.hpp"

#include <cassert>
#include <cmath> // sqrt, pow?
//!!NOT YET! Too cumbersome for the trivial alternative.
//!!#include <optional>
//!!	using std::optional, std::nullopt;
#include <map>
#include <string>
	using std::stoi, std::stof;
//#include <cstdlib> // strtof
#include <utility>
	using std::move;

#include <iostream>
	using std::cerr;


namespace Model {

using namespace std;
using namespace Szim;
using namespace Math;


//============================================================================
World::World() :
	gravity_mode(Normal)
{
}

//----------------------------------------------------------------------------
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
IPROF_FUNC;
	assert(bodies.size() > 0);
	assert(ndx != (size_t)-1);
	bodies.erase(bodies.begin() + ndx);
}


//============================================================================
void World::update(float dt, SimApp& game)
//!! Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
//----------------------------------------------------------------------------
#define _LEGACY_GRAVITY_CALC_ // -> #65

#define _SKIP_SOME_STATE_UPDATES_ 10
	// Slow changes (like cooling bodies) don't need updates in every frame
	//!! Normalize for frame rate! (Needs the avg. frame-time as input then!)

//#define _SKIP_SOME_INTERACTIONS_ 3
	//!! Should spread the "skippage" across varied blocks of entities
        //!! not just entirely skipping some frames for all (-> very jittery!)
//----------------------------------------------------------------------------
IPROF_FUNC;

	if (dt == 0.f) { // (Whatever the accuracy of this, good enough.)
		return;  // <- This may change later (perhaps selectively,
		         // for any "timeless features"), but for now: fix #298!
	}

#ifdef _SKIP_SOME_STATE_UPDATES_
//!!testin' testin'...
static int skipping_T_recalc = _SKIP_SOME_STATE_UPDATES_ * 10;
static float accumulated_skip_dt = 0; // s
auto last_dt = dt;
#endif

#ifdef _SKIP_SOME_INTERACTIONS_
static int skipping_n_interactions = _SKIP_SOME_INTERACTIONS_;
#endif

	// Go through autogenic effects first:
	//!!This should be a separate collection tho; super wasteful to go thru all every time!
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Lifecycle mgmt... -- mark and skip decaying objects:
		if (body->can_expire()) {
			body->lifetime -= dt;
			if (body->lifetime <= 0) {
				body->terminate(); // Set to a well-defined state that's easy to check later!
				body->on_event(Event::Terminated);

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
			//!! Well, OK, doing a "skiprate" refresh here, so that's gonna be the condition then...
#ifdef _SKIP_SOME_STATE_UPDATES_
			if (!--skipping_T_recalc) {
				skipping_T_recalc = _SKIP_SOME_STATE_UPDATES_ * 10;
				body->recalc();
//cerr << "#"<<i <<" Recalculated (T = " << body->T << ").\n";
			}
#endif
//cerr << "#"<<i <<" Cooled to " << body->T << "...\n";
		}

		// Thrust -- for objects with (working) thrusters...:
		if (body->has_thruster()) {
			Vector2f F_thr( (-body->thrust_left.thrust_level() + body->thrust_right.thrust_level()) * dt,
			                ( body->thrust_up.thrust_level()   - body->thrust_down.thrust_level() ) * dt);
			body->v += (F_thr / body->mass);
		}
	}

#if defined(_SKIP_SOME_STATE_UPDATES_) && defined(_SKIP_SOME_INTERACTIONS_)
//!!testin' testin'...
if (_SKIP_SOME_STATE_UPDATES_ && --skipping_n_interactions) {
	accumulated_skip_dt += dt; goto end_interact_loop;
} else {
//	cerr << ".";
	// Consume the time we've "collected" while skipping:
	if (accumulated_skip_dt > 0) { dt = accumulated_skip_dt; accumulated_skip_dt = 0; }
	// Start skipping again:
	skipping_n_interactions = _SKIP_SOME_INTERACTIONS_;
}
#endif

// Now do the interaction matrix:
//!!This line below is hard-coded to globe-ndx == 0, and also ignores any other (potential) players!...
for (size_t actor_obj_ndx = 0; actor_obj_ndx < (_interact_all ? bodies.size() : 1); ++actor_obj_ndx)
{
	if (bodies[actor_obj_ndx]->terminated())
		continue;

//#ifdef _LEGACY_GRAVITY_CALC_
//	for (size_t i = 0; i < bodies.size(); ++i)
//#else
//	for (size_t i = actor_obj_ndx + 1; i < bodies.size(); ++i)
//#endif
	assert(gravity_mode == Normal || gravity_mode == Skewed || gravity_mode == Off);
	for (size_t i = gravity_mode == Normal ? 0 : actor_obj_ndx + 1; i < bodies.size(); ++i)
	{
		auto& body = bodies[i]; //!!?? shared_ptr... worth the &?

		if (body->terminated()) continue;

		// Collisions & gravity...
		//!! see the relative looping now! if (i != actor_obj_ndx)
		{
			auto& other = bodies[actor_obj_ndx];

			auto dx = other->p.x - body->p.x,
			     dy = other->p.y - body->p.y;

//			auto distance = Math::distance2(body->p.x, body->p.y, other->p.x, other->p.y);
			auto distance = Math::mag2(dx, dy);

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
//!!			auto rel_dv = distance2(body->v.x, body->v.y, other->v.x, other->v.y);
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

					//!! Handle this in a hook:
					//body->v = {0, 0}; // - or bounce, or stick to the other body and take its v, or any other sort of interaction...

					// Interestingly, if no speed reset/change is applied at all,
					// an orbiting moon that hits the surface of its attractor with
					// a flat-enough angle, STILL DOES APPEAR TO BOUNCE OFF OF IT! :-O
					// Why? -> #56
					// a) Since the speed will not get updated while travelling inside
					//    the attractor body (i.e. still colliding), for a shallow
					//    "surface rub" collision, that's a pretty damn' good approx.
					//    of a proper collision calc. anyway.
					// b) The viewer would expect a continued bending (spiralling) of
					//    the trajectory if not colliding, which would abruptly stop
					//    happening on a collision, creating the illusion of a bounce.
					// c) With a shallow angle the collision takes short enough for
					//    this fake (non-)calculation to remain unnoticeable.
				}

				//! Also call a high-level, "predefined emergent interaction" hook:
				game.interaction_hook(this, Event::Collided, body.get(), other.get());

			} else if (!body->superpower.gravity_immunity) { // process gravity if not colliding
//#ifdef _LEGACY_GRAVITY_CALC_
if (gravity_mode == Normal) {
//!!				float g = Physics::G * other->mass / (distance * distance);
				float g = gravity * other->mass / (distance * distance);
				Vector2f gvect(dx * g, dy * g);
				//!!should rather be: Vector2f gvect(dx / distance * g, dy / distance * g);
				Vector2f dv = gvect * dt;
				body->v += dv;
//#else //!! #65...:
} else if (gravity_mode == Skewed) {
//!!				float G_per_DD = Physics::G / (distance * distance);
				float G_per_DD = gravity / (distance * distance);
				// New accel. of the "inner" body:
				float a1 = G_per_DD * other->mass;
				//!!should rather be: Vector2f gvect(dx / distance * g, dy / distance * g);
				Vector2f dv1 = Vector2f{dx * a1, dy * a1} * dt;
				body->v += dv1;
/*
				// New accel. of the "outer" body:
				float a2 = -G_per_DD * body->mass;
				//!!should rather be: Vector2f gvect(dx / distance * g, dy / distance * g);
				Vector2f dv2 = Vector2f{dx * a2, dy * a2} * dt;
				other->v += dv2;
*/
//#endif
} else {
	assert(gravity_mode == Off);
}

//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<body->v.x<<","<<body->v.y<<") " << endl;
			}
		} // if interacting with itself

/*!! Very interesting magnified effect if calculated here, esp. with negative friction -- i.e. an expanding universe:
		// Friction:
		Vector2f dv = friction_decel * (dt);
		Vector2f friction_decel(-body->v.x * friction, -body->v.y * friction);
		body->v += dv;
!!*/		
//cerr << "v["<<i<<"] = ("<<body->v.x<<","<<body->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
	} // inner for
} // outer for

#ifdef _SKIP_SOME_INTERACTIONS_
end_interact_loop:
dt = last_dt; // Restore "real dt" for calculations outside the "skip cheat"!
#endif

	// All-inclusive postprocessing loop for friction [but why here? test what diff it makes if done in the pre-interact. loop],
	// and actually updating the positions finally
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto& body = bodies[i];

		// Friction:
		Vector2f friction_decel(-body->v.x * friction, -body->v.y * friction);
		Vector2f dv = friction_decel * dt;
		body->v += dv;
		
		// And finally the positions:
		Vector2f ds(body->v.x * dt, body->v.y * dt);
		body->p += ds;
	}
}


//----------------------------------------------------------------------------
void World::_copy(World const& other)
{
//cerr << "World copy requested!\n";
	if (&other != this)
	{
		//!! Move these into some props container to prevent forgetting
		//!! some, when manip. them one by one! :-/
		//!! There might anyway be a distinction between these and
		//!! throw-away volatile state (like caches) in the future.
		friction = other.friction;
		_interact_all = other._interact_all;
		gravity_mode = other.gravity_mode;
		gravity = other.gravity;

		bodies.clear();
		for (const auto& b : other.bodies) {
			add_body(*b);
		}
	}
}

} // namespace Model