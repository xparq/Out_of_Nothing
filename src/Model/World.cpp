// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "Model/World.hpp"

//!! Shrink even this dependency: don't include the entire fucking SimApp,
//!! only its modelling-related services that are actually used:
#include "Szim/SimApp.hpp"

#include "Szim/diag/Log.hpp"
//!!??#include <iostream> //!!?? Why does Log.hpp not get it (even the fake one with SZ_LOG_REPLACE_IOSTREAM)?! :-o
//!!??	using std::cerr;

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


namespace Model {

using namespace std;
using namespace Szim;
using namespace Math;


//============================================================================
/*static*/ const World::Properties World::Default_Props =
{
	.friction       = 0.03f, //!!Take its default from the cfg instead!
	.gravity_mode   = GravityMode::Default,
	.gravity        = Phys::G, //!!Take its default from the cfg instead!
	.interact_n2n   = false,
	.repulsion_stiffness = 0, // 0.0000000005 - 0.000000001
	.collision_mode = CollisionMode::Glide_Through,
};


//============================================================================
World::World()
	: props(Default_Props)
	, loop_mode(LoopMode::Default)
{
}

//----------------------------------------------------------------------------
EntityID World::add_body(Entity const& obj)
{
ZoneScoped; //!!IPROF("add_body-copy");
	bodies.push_back(std::make_shared<Entity>(obj));
	auto ndx = bodies.size() - 1;
	bodies[ndx]->recalc();
	return ndx;
}

EntityID World::add_body(Entity&& obj)
{
ZoneScoped; //!!IPROF("add_body-move");
	obj.recalc(); // just recalc the original throw-away obj
	bodies.emplace_back(std::make_shared<Entity>(obj));
	return bodies.size() - 1;
}

void World::remove_body(EntityID ndx)
{
ZoneScoped;
	assert(bodies.size() > 0);
	assert(ndx != Entity::NONE);
	bodies.erase(bodies.begin() + ndx);
}


//============================================================================
void World::init(Szim::SimApp& app)
{
	app.init_world_hook();
	//!! See e.g. OONApp::init() in OON.cpp, and init_world() in SimApp.hpp why this
	//!! convoluted inverted control, instead of the engine just calling the app's
	//!! init_world()!... (E.g. no virtual dispatch in the legacy init, etc...)
}

void World::update(float dt, SimApp& app)
// The updates are deterministic currently (but random effects may be added later).
{
//!!?? LOGE << app.world().props.repulsion_stiffness;

	//!! pre_update_hook(); // Per-tick model housekeeping before updating
	update_intrinsic(dt, app); // Advance entity states independently (O(n), can be parallel)
	update_pairwise(dt, app);  // Let them talk... (O(n²), probably (inherently?) sequential!)
	update_global(dt, app);    // Apply global effects (O(n), can be parallel)
	//!! post_update_hook(); // // Housekeeping after the update — i.e. before drawing!
}

//----------------------------------------------------------------------------
#define _INTRINSIC_UPDATE_SKIP_COUNT_ 10
	// Slow changes (like cooling) don't need updates in every frame
	//!! Normalize for frame rate! (Needs the avg. frame-time as input then!)

//#define _PAIRWISE_UPDATE_SKIP_COUNT_ 3
	//!! Should spread the "skippage" across varied blocks of entities
        //!! not just entirely skipping some frames for all (-> jitter!)


//----------------------------------------------------------------------------
void World::update_intrinsic(float dt, Szim::SimApp& app [[maybe_unused]])
//----------------------------------------------------------------------------
{
ZoneScoped;

	if (dt == 0.f) { // (Whatever the accuracy of this, good enough.)
		return;  // <- This may change later (perhaps selectively,
		         // for any "timeless features"), but for now: fix #298!
	}

#ifdef _INTRINSIC_UPDATE_SKIP_COUNT_
//!!testin' testin'...
static int skipping_T_recalc = _INTRINSIC_UPDATE_SKIP_COUNT_ * 10;
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
#ifdef _INTRINSIC_UPDATE_SKIP_COUNT_
			if (!--skipping_T_recalc) {
				skipping_T_recalc = _INTRINSIC_UPDATE_SKIP_COUNT_ * 10;
				body->recalc();
//cerr << "#"<<i <<" Recalculated (T = " << body->T << ").\n";
			}
#endif
//cerr << "#"<<i <<" Cooled to " << body->T << "...\n";
		}

		// Thrust -- for objects with (working) thrusters...:
		if (body->has_thruster()) {
			Phys::Force2 F_thr(
				(-body->thrust_left.thrust_level() + body->thrust_right.thrust_level()) * dt,
				( body->thrust_up.thrust_level()   - body->thrust_down.thrust_level() ) * dt
			);
			//!! Not physically correct, but a "numerically OK" shortcut:
			body->v += Phys::Velo2(F_thr / body->mass);
		}
	}

#if defined(_INTRINSIC_UPDATE_SKIP_COUNT_) && defined(_PAIRWISE_UPDATE_SKIP_COUNT_)
//!!testin' testin'...
if (_INTRINSIC_UPDATE_SKIP_COUNT_ && --skipping_n_interactions) {
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
void World::update_pairwise(float dt, Szim::SimApp& app)
//----------------------------------------------------------------------------
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
for (size_t source_obj_ndx = 0;
            source_obj_ndx < (props.interact_n2n ? obj_cnt : 1); //!!?? ... : app.number_of_players()
		//!! Hard-coded to player-entity-index == 0, and ignores any other (potential) players!...
		//!![WHAT DID I MEAN BY THIS:] That 1 is incompatible with `interact_n2n` actually! Should be 0, and 1 only with `interact_playeronly`!
          ++source_obj_ndx)
{
	using enum GravityMode;

	if (bodies[source_obj_ndx]->terminated())
		continue;

#ifdef _MSC_VER
//# pragma omp simd //!!Well, hilariously, this (or the outer?) makes it slightly SLOWER! :) :-o
#endif

	// Iterate over every pair...
	//
	// - Full-matrix mode: iterate through the full matrix, doing one interaction calc. per step,
	//                     letting the iteration order determine the direction of the interaction.
	//                     NOTE: this would call the interaction filter (e.g. a collision check)
	//                     twice, to support ordered relations: once for (a, b) and for (b, a)!
	// - Half-matrix mode: traverse only half of the matrix (split along the diagonal).
	//                     Note: the model may still support ordered interactions in this mode,
	//                     by doing two interaction calcs. (one for each direction) in every step!
	//                     The "interaction filter" (e.g. collision check) will only be called
	//                     once per pair.
	//
	for (size_t target_obj_ndx = loop_mode == LoopMode::Full_Matrix ? 0 : source_obj_ndx + 1;
	            target_obj_ndx < obj_cnt;
	          ++target_obj_ndx)
	{
	//!!IPROF("Interact. cycle"); //! SLOWS DOWN THE MODEL HORRIBLY! (Was 0.22-0.27 μs, incl. the meas. overh.)

		if (source_obj_ndx == target_obj_ndx) continue; // Skip self...

		Entity* target = bodies[target_obj_ndx].get(); //! shared_ptr
		if (target->terminated()) continue; // Skip if dead...

		Entity* source = bodies[source_obj_ndx].get();
//!!?? Why was this not here:
//!!??		if (source->terminated()) continue; // Skip if dead...

		//
		// Motion: collision, gravity...
		//

		// Calc. the distance first (needed for everything...):
		auto dist_vect = source->p - target->p;
		auto dist_normal = dist_vect.normalized();
		auto distance = dist_vect.length();

		// Then a distance-dependent, but mass-independent "gravity falloff" factor,
		// as a precalculated const. for this loop cycle:
		NumType g_factor;
		switch (props.gravity_mode)
		{
		case Hyperbolic:
			g_factor = props.gravity / distance; break;
		case Realistic:
		case Experimental: //!! Some old saved states still have this for Realistic!
			g_factor = props.gravity / (distance * distance); break;
		default:
			assert(props.gravity_mode == Off);
			g_factor = 0; break;
		}

		//! Collision det. is crucial also for preventing 0 distance to divide by!
		//!!Collision and "distance == 0" are not the same here:
		//!!distance is measured from the center-of-mass points, not the surfaces!
		//!!But d = 0 is a "subset" of collisions, so if there would be a "safe"
		//!!adjustment for any collision to guarantee distance != 0, that would be
		//!!enough...

		//!!Also, obviously, "almost 0" could be just as bad as 0 (which is not
		//!!even quite real in float, and would anyway just yield a NaN), so
		//!!the real solution would be smarter modeling of the high-speed case
		//!!to do what nature does... Dunno, create a black hole, if you must! :)

		//!! Remember to avoid repeating the check in Full_Matrix mode, if it's not actually directional!
		if (//!!gravity_mode != Realistic && //!!TEMP HACK for #252: allow accel. while overlapping
			/*physics.*/is_colliding(target, source, distance)) {

			// If the collision results in a well-defined fixed position/velocity etc.,
			// they may need special treatment, because at this point the checked body may not
			// yet have reached (or crossed the boundary of) the other, so it needs to be adjusted
			// to the expected collision end-state!
			//!!/*physics.*/collide_hook(target, source, distance);

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
				//target->p = source->p + Pos2<NumType>{1, 1};

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

			if (loop_mode == LoopMode::Full_Matrix) { // #65... Separate cycles for the two halves of the interaction is 10-12% SLOWER! :-o

				if (!target->superpower.gravity_immunity) {
					//!! Note: doing it branchless, i.e. multiplying with the bool flag (as 0 or 1)
					//!! made it significantly *SLOWER*! :-o
					NumType a = g_factor * source->mass;
					target->v += Phys::Velo2{dist_normal * a * NumType(dt)};
		//			auto dv = Phys::Velo2{dist_normal * a} * NumType(dt);
					//! Optimizing ...*dt into a dv scaling factor (a*dt) resulted in
					//! rounding errors & failed regression testing... [With float or double?!]
					//! But should be fine:
		//			float _dvscale = gravity * source->mass / (distance*distance) * dt;
		//			auto _dv = Velo2(dx * _dvscale, dy * _dvscale);
		/*!
		{static bool done=false;if(!done){done=true; // These look the same, but the calculations differ! :-o
		cerr << "dv : "<<  dv.x <<", "<<  dv.y <<"\n";
		cerr << "_dv: "<< _dv.x <<", "<< _dv.y <<"\n"; }}
		!*/
		//			target->v += dv; //! += _dv;

		/*!!
		if(((OONApp&)game).controls.ShowDebug) {
		//!!This is useless, while the event loop is stalled, as the update loop here
		//!!would just blast through this a million times with the old input state! :-/
		cerr << "a: "<< a_target <<", distance: "<< distance <<"\n"; // It was like 0.x with 3 bodies in close proximity, and barely moving.
		}
		!!*/
				}

			} else { // loop_mode == Half (-> #65)
//!! TESTING:
//props.repulsion_stiffness = 0.00000001; // Too strong
//props.repulsion_stiffness = 0.000000001; // Viable (but a bit strong)
//props.repulsion_stiffness = 0.0000000008; // Viable
//props.repulsion_stiffness = 0.0000000006; // Viable (but a bit weak)
//props.repulsion_stiffness = 0.0000000001; // Too weak
	//!!?? Why does this non-zero REPULSIVE force *mitigate* the initial big bang?! :-o

				double sculptor_field_correction;
				if (props.repulsion_stiffness == 0) {
					sculptor_field_correction = 0;
				} else {
					sculptor_field_correction =
						g_factor * props.repulsion_stiffness
						* (source->mass / distance)
						* (target->mass / distance)
					;
				}

				if (!target->superpower.gravity_immunity) {
					auto a =  g_factor * source->mass
						- sculptor_field_correction;
					target->v += Phys::Velo2{dist_normal * a * dt};
				}
				if (!source->superpower.gravity_immunity) {
					auto a = - (g_factor * target->mass
						- sculptor_field_correction);
					source->v += Phys::Velo2{dist_normal * a * dt};
				}
			}

//cerr << "gravity pull on ["<<i<<"]: dist = "<<distance << ", g = "<<g << ", gv = ("<<target->v.x<<","<<target->v.y<<") " << endl;

		} // Process gravity?


/*!! Very interesting magnified effect if calculated here, esp. with negative friction (i.e. an expanding universe):
		// Friction:
		V2f dv = friction_decel * (dt);
		V2f friction_decel(-target->v.x * friction, -target->v.y * friction);
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
void World::update_global(float dt, Szim::SimApp& app)
//----------------------------------------------------------------------------
{
	// All-inclusive postprocessing loop for friction [but why here? test what diff it makes if done in the pre-interact. loop],
	// and actually updating the positions finally
	for (size_t obj_cnt = bodies.size(), i = 0; i < obj_cnt; ++i)
	{
		auto& body = bodies[i];

		// Friction - adjust velocities:
		auto friction_decel = body->v * NumType(props.friction);
		body->v -= friction_decel * NumType(dt);
		
		// And finally, the positions:
/*!! This gets stuck even after immediately releasing the Alt keys, and just keeps dumping to the console (e.g. 2 x 515 lines)!...
if(((OONApp&)app).controls.ShowDebug) {
	cerr << "#"<<i<<": moving this much: "<< body->v.x * dt <<", "<< body->v.x <<"\n";
}
!!*/
		body->p += body->v * NumType(dt);
	}
}


//----------------------------------------------------------------------------
//!! Seen a 147 -> 131 FPS drop by moving these to the .cpp! :-o
bool World::is_colliding([[maybe_unused]] const Entity* obj1, [[maybe_unused]] const Entity* obj2) const
//!! Should take the body shape into account.
{
	//auto distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));
	return false;
}

bool World::is_colliding(const Entity* obj1, const Entity* obj2, Phys::Length distance) const
// Only for circles yet!
{
	return props.collision_mode == CollisionMode::Off
		? false //!! #526: make the resulting "explosive decay" a feature!
		: distance <= obj1->r + obj2->r;
}


//----------------------------------------------------------------------------
void World::_copy(World const& source)
{
//cerr << "World copy requested!\n";
	if (&source != this)
	{
		props = source.props;

		bodies.clear();
		for (const auto& b : source.bodies) { add_body(*b); }

		loop_mode = source.loop_mode;
	}
}

} // namespace Model