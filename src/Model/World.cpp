//#define DISABLE_FULL_INTERACTION_LOOP // Only helps ~0.5% with 500 bodies...

// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "Model/World.hpp"

//!! Shrink even this dependency: don't include the entire fucking SimApp,
//!! only its modelling-related services that are actually used:
#include "Engine/SimApp.hpp"

//!!OTOH, when the World will get finally split into an abstract base and the implem.,
//!! the implem. part would be free to know the app! So, for now, right here:
#include "app/OON.hpp"
	using namespace OON;

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
	gravity_mode(GravityMode::Default),
	loop_mode(LoopMode::Default)
{
}

//----------------------------------------------------------------------------
size_t World::add_body(Body const& obj)
{
ZoneScoped; //!!IPROF("add_body-copy");
	bodies.push_back(std::make_shared<Body>(obj));
	auto ndx = bodies.size() - 1;
	bodies[ndx]->recalc();
	return ndx;
}

size_t World::add_body(Body&& obj)
{
ZoneScoped; //!!IPROF("add_body-move");
	obj.recalc(); // just recalc the original throw-away obj
	bodies.emplace_back(std::make_shared<Body>(obj));
	return bodies.size() - 1;
}

void World::remove_body(size_t ndx)
{
ZoneScoped;
	assert(bodies.size() > 0);
	assert(ndx != (size_t)-1);
	bodies.erase(bodies.begin() + ndx);
}


//============================================================================
void World::init(Szim::SimApp& app)
{
	app.init_world_hook();
}

void World::update(float dt, SimApp& app)
//!! Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	update_before_interactions(dt, app);
	update_pairwise_interactions(dt, app);
	update_after_interactions(dt, app);
}

//----------------------------------------------------------------------------
#define _AUTOGENIC_UPDATE_SKIP_COUNT_ 10
	// Slow changes (like cooling) don't need updates in every frame
	//!! Normalize for frame rate! (Needs the avg. frame-time as input then!)

//#define _PAIRWISE_UPDATE_SKIP_COUNT_ 3
	//!! Should spread the "skippage" across varied blocks of entities
        //!! not just entirely skipping some frames for all (-> jitter!)


//----------------------------------------------------------------------------
void World::update_before_interactions(float dt, Szim::SimApp& app [[maybe_unused]])
{
ZoneScoped;

	if (dt == 0.f) { // (Whatever the accuracy of this, good enough.)
		return;  // <- This may change later (perhaps selectively,
		         // for any "timeless features"), but for now: fix #298!
	}

#ifdef _AUTOGENIC_UPDATE_SKIP_COUNT_
//!!testin' testin'...
static int skipping_T_recalc = _AUTOGENIC_UPDATE_SKIP_COUNT_ * 10;
static float accumulated_skip_dt = 0; // s
auto last_dt = dt;
#endif

#ifdef _PAIRWISE_UPDATE_SKIP_COUNT_
static int skipping_n_interactions = _PAIRWISE_UPDATE_SKIP_COUNT_;
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
#ifdef _AUTOGENIC_UPDATE_SKIP_COUNT_
			if (!--skipping_T_recalc) {
				skipping_T_recalc = _AUTOGENIC_UPDATE_SKIP_COUNT_ * 10;
				body->recalc();
//cerr << "#"<<i <<" Recalculated (T = " << body->T << ").\n";
			}
#endif
//cerr << "#"<<i <<" Cooled to " << body->T << "...\n";
		}

		// Thrust -- for objects with (working) thrusters...:
		if (body->has_thruster()) {
			Vector2<NumType> F_thr(
				(-body->thrust_left.thrust_level() + body->thrust_right.thrust_level()) * dt,
				( body->thrust_up.thrust_level()   - body->thrust_down.thrust_level() ) * dt
			);
			body->v += (F_thr / body->mass);
		}
	}

#if defined(_AUTOGENIC_UPDATE_SKIP_COUNT_) && defined(_PAIRWISE_UPDATE_SKIP_COUNT_)
//!!testin' testin'...
if (_AUTOGENIC_UPDATE_SKIP_COUNT_ && --skipping_n_interactions) {
	accumulated_skip_dt += dt; goto end_interact_loop;
} else {
//	cerr << ".";
	// Consume the time we've "collected" while skipping:
	if (accumulated_skip_dt > 0) { dt = accumulated_skip_dt; accumulated_skip_dt = 0; }
	// Start skipping again:
	skipping_n_interactions = _PAIRWISE_UPDATE_SKIP_COUNT_;
}
#endif
}

//----------------------------------------------------------------------------
void World::update_pairwise_interactions(float dt, Szim::SimApp& app)
{
// Now do the interaction matrix:
//!!
//!! PROTECT AGAINST OTHER THREADS POTENTIALLY ADDING/DELETING OBJECTS!
//!!

//!!--------------------------------------------------------------------------
//!! JUST FOR A REMINDER, NOTHING SERIOUS (OR EVEN MEANINGFUL) YET!
//!! (E.g. the loops may well be just unsuitable for such optim. as-is.)
//!!
// Compiler boost:
// - MSVC:
//   Overall vectorization & floating-point loop optim.
//   https://learn.microsoft.com/en-us/cpp/parallel/openmp/openmp-simd?view=msvc-170
//   FTR: No significant diff.; /fp:fast was a lot more immediately noticable.
//        In fact, adding /openmp on top of /fp:fast *DEGRADED* the performance! :-o
//
#ifdef _MSC_VER
//# pragma vector // CPP2022+ (mine can't do it)
//# pragma ivdep  // CPP2022+ (mine can't do it)
//# pragma omp simd // Must be put directly before a `for`!
//!! ... simdlen(8): which version? mine can't do it
#endif
//!!
//!!--------------------------------------------------------------------------

auto obj_cnt = bodies.size();
#ifdef _MSC_VER
//# pragma omp simd //!!Well, hilariously, this (or the inner?) makes it slightly SLOWER! :) :-o
#endif
for (size_t source_obj_ndx = 0; source_obj_ndx < (_interact_all ? obj_cnt : 1); ++source_obj_ndx)
	//!! That 1 is incompatible with `interact_all` actually! Should be 0, and 1 only with `interact_playeronly`!
	//!! Hard-coded to player-entity-index == 0, and ignores any other (potential) players!...
{
	using enum GravityMode;

	if (bodies[source_obj_ndx]->terminated())
		continue;

#ifdef _MSC_VER
//# pragma omp simd //!!Well, hilariously, this (or the outer?) makes it slightly SLOWER! :) :-o
#endif
#ifndef DISABLE_FULL_INTERACTION_LOOP
	// Iterate over the pairs both ways in Full mode...
	for (size_t target_obj_ndx = loop_mode == LoopMode::Full ? 0 : source_obj_ndx + 1;
		target_obj_ndx < obj_cnt; ++target_obj_ndx)
#else
	// Iterate over the pairs once only, doubling any bidirect. interactions inside the inner cycle!
	for (size_t target_obj_ndx = source_obj_ndx + 1;
		target_obj_ndx < obj_cnt; ++target_obj_ndx)
#endif
	{
	//!!IPROF("Interact. cycle"); //! SLOWS DOWN THE MODEL HORRIBLY! (Was 0.22-0.27 μs, incl. the meas. overh.)

		if (source_obj_ndx == target_obj_ndx) continue; // Skip itself...

		Body* target = bodies[target_obj_ndx].get(); //! shared_ptr
		if (target->terminated()) continue;

		Body* source = bodies[source_obj_ndx].get();

		// Collisions & gravity...
		//!! see the relative looping now! if (i != source_obj_ndx)
		{
			auto dx = source->p.x - target->p.x,
			     dy = source->p.y - target->p.y;

//			auto distance = Math::distance2(target->p.x, target->p.y, source->p.x, source->p.y);
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

			//!! Avoid repeating the check in ordered-interactions mode, with double iterations:
			if (//!!gravity_mode != Realistic && //!!TEMP HACK for #252: allow accel. while overlapping
				/*physics.*/is_colliding(target, source, distance)) {
			//! Done before actually reaching the body (so we can avoid the occasional frame showing the penetration :) )!
			//  (Opting for the perhaps even less natural "but didn't even touch!" issue...)

				// If the collision results in a well-defined fixed position/velocity etc.,
				// they may need special treatment, because at this point the checked body may not
				// yet have reached (or crossed the boundary of) the other, so it needs to be adjusted
				// to the expected collision end-state!
				//!!/*physics.*/collide_hook(target, source, distance);
				//! Call God, too:

				// Call this before processing the collision-induced speed changes,
				// so that the calc. can take into consideration the relative speed!
//!!			auto rel_dv = distance2(target->v.x, target->v.y, source->v.x, source->v.y);
				static constexpr float EPS_COLLISION = CFG_GLOBE_RADIUS/10; //!! experimental guesstimate (was: 100000); should depend on the relative speed!
				if (abs(distance - (target->r + source->r)) < EPS_COLLISION ) {
//cerr << "Touch!\n";
					if (!app.touch_hook(this, target, source)) {
						;
					}
				} else {
//cerr << " - Collided, but NO TOUCH. d = " << distance << ", delta = "<<abs(distance - (target->r + source->r)) << " (epsilon = "<<EPS_COLLISION<<")\n";
				}

				// Note: calling the hook before processing the collision!
				// If the listener returns false, it didn't process it, so we should.
				if (!app.collide_hook(this, target, source, distance)) {

					//!! Handle this in a hook:
					//target->v = {0, 0}; // - or bounce, or stick to the other body and take its v, or any other sort of interaction...

					// Swallow the target -- for testing free-fall & accretion:
					//target->v = source->v;
					//target->p = source->p + Vector2<NumType>{1, 1};

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
//!!...				app.undirected_interaction_hook(this, Event::Collided, source, target, dt, distance);

			} else { // process gravity if not colliding
				//!!app.directed_interaction_hook(this, source, target, dt, distance);
					//!! Wow, this fn. call costs an FPS drop from ~175 to ~165 with 500 objs.! :-/
#ifndef DISABLE_FULL_INTERACTION_LOOP
if (loop_mode == LoopMode::Full) { // #65... Separate cycles for the two halves of the interaction is 10-12% SLOWER! :-o
	switch (gravity_mode) {
	case Hyperbolic: // #65... Separate cycles for the two halves of the interaction is 10-12% SLOWER! :-o

				if (!target->superpower.gravity_immunity) {
				  //! Note: doing it branchless, i.e. multiplying with the bool flag (as 0 or 1)
				  //! made it significantly _slower_! :-o
					NumType a = gravity * source->mass / (distance * distance);
					auto dv = Vector2<NumType>{dx * a, dy * a} * NumType(dt); // #525: dx/distance, dy/distance...
					//! Optimizing ...*dt into a dv scaling factor (a*dt) resulted in
					//! rounding errors & failed regression testing... But should be fine:
//					float _dvscale = gravity * source->mass / (distance*distance) * dt;
//					auto _dv = Vector2<NumType>(dx * _dvscale, dy * _dvscale);
/*!
{static bool done=false;if(!done){done=true; // These look the same, but the calculations differ! :-o
cerr << "dv : "<<  dv.x <<", "<<  dv.y <<"\n";
cerr << "_dv: "<< _dv.x <<", "<< _dv.y <<"\n"; }}
!*/
					target->v += dv; //! += _dv;
				}
		break;
	case Realistic:
	case Experimental:
				if (!target->superpower.gravity_immunity) {
					NumType a = gravity * source->mass / (distance * distance); //!!?? distance^3 too big for the divider?
					auto dv = Vector2<NumType>{dx * a, dy * a} * NumType(dt/distance); // #525: dx/distance, dy/distance...
					target->v += dv;
/*!!
if(((OONApp&)game).controls.ShowDebug) {
	//!!This is useless, while the event loop is stalled, as the update loop here
	//!!would just blast through this a million times with the old input state! :-/
	cerr << "a: "<< a_target <<", distance: "<< distance <<"\n"; // It was like 0.x with 3 bodies in close proximity, and barely moving.
}
!!*/
				}
		break;
//!!	case Experimental:
//!!		assert(gravity_mode == Experimental);
//!!		break;
	default:
		assert(gravity_mode == Off);
		break;
	} // switch (gravity_mode)

} else { // loop_mode == Half (-> #65)
#endif // DISABLE_FULL_INTERACTION_LOOP

	//!! Do the same switch here, too!

		if (gravity_mode == Hyperbolic) {
				const NumType G_dt_div_d2 = gravity / (distance * distance) * dt;
				if (!target->superpower.gravity_immunity) {
					auto a = G_dt_div_d2 * source->mass;
					auto dv = Vector2<NumType>{dx * a, dy * a};
					target->v += dv;
				}
				if (!source->superpower.gravity_immunity) {
					auto a = -G_dt_div_d2 * target->mass;
					auto dv = Vector2<NumType>{dx * a, dy * a};
					source->v += dv;
				}
		} else if (gravity_mode == Realistic) {
				const NumType G_dt_div_d2 = gravity / (distance * distance) * dt; //!!?? distance^3 too big for the divider?
				if (!target->superpower.gravity_immunity) {
					auto a = G_dt_div_d2 * source->mass;
					auto dv = Vector2<NumType>{dx * a, dy * a} * NumType(dt/distance); // #525: dx/distance, dy/distance...
					target->v += dv;
				}
				if (!source->superpower.gravity_immunity) {
					auto a = -G_dt_div_d2 * target->mass;
					auto dv = Vector2<NumType>{dx * a, dy * a} * NumType(dt/distance); // #525: dx/distance, dy/distance...
					source->v += dv;
				}
		}


#ifndef DISABLE_FULL_INTERACTION_LOOP
}
#endif

//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<target->v.x<<","<<target->v.y<<") " << endl;
			}
		} // if interacting with itself

/*!! Very interesting magnified effect if calculated here, esp. with negative friction -- i.e. an expanding universe:
		// Friction:
		Vector2f dv = friction_decel * (dt);
		Vector2f friction_decel(-target->v.x * friction, -target->v.y * friction);
		target->v += dv;
!!*/		
//cerr << "v["<<i<<"] = ("<<target->v.x<<","<<target->v.y<<"), " << " dx = "<<ds.x << ", dy = "<<ds.y << ", dt = "<<dt << endl;
	} // inner loop (of targets)
} // outer loop (of soucres)

#ifdef _PAIRWISE_UPDATE_SKIP_COUNT_
end_interact_loop:
dt = last_dt; // Restore "real dt" for calculations outside the "skip cheat"!
#endif
}

//----------------------------------------------------------------------------
void World::update_after_interactions(float dt, Szim::SimApp& app)
{
	// All-inclusive postprocessing loop for friction [but why here? test what diff it makes if done in the pre-interact. loop],
	// and actually updating the positions finally
	for (size_t obj_cnt = bodies.size(), i = 0; i < obj_cnt; ++i)
	{
		auto& body = bodies[i];

		// Friction - adjust velocities:
		auto friction_decel = body->v * NumType(friction);
		body->v -= friction_decel * NumType(dt);
		
		// And finally, the positions:
///*!!
if(((OONApp&)app).controls.ShowDebug) {
	cerr << "#"<<i<<": moving this much: "<< body->v.x * dt <<", "<< body->v.x <<"\n";
}
//!!*/
		body->p += body->v * NumType(dt);
	}
}


//----------------------------------------------------------------------------
void World::_copy(World const& source)
{
//cerr << "World copy requested!\n";
	if (&source != this)
	{
		//!! Move these into some props container to prevent forgetting
		//!! some, when manip. them one by one! :-/
		//!! There might anyway be a distinction between these and
		//!! throw-away volatile state (like caches) in the future.
		gravity = source.gravity;
		friction = source.friction;
		gravity_mode = source.gravity_mode;
		loop_mode = source.loop_mode;
		_interact_all = source._interact_all;

		bodies.clear();
		for (const auto& b : source.bodies) {
			add_body(*b);
		}
	}
}

} // namespace Model