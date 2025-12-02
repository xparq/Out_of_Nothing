#ifndef _AMWDMC$SDFY7JM8FM4F60YUCMFV7NH489TYU_
#define _AMWDMC$SDFY7JM8FM4F60YUCMFV7NH489TYU_

#include "Szim/Model/Meta.hpp"
#include "Szim/Model/Entity.hpp"
//!!??#include "Physics.hpp"
#include "sz/lang/IGNORE.hh"

//!!#include <memory>     // shared_ptr
//!!#include <vector>
//!!No, not yet; just too cumbersome for too little gain:
//!!#include <optional> // for load()
//!!#include <iosfwd> // for save/load


namespace Szim {

class SimApp;

namespace Model::Core {

// World coordinate system (Right-handed, like OpenGL's):
//
//         +y  -z
//          | /
//          |/
//  -x ---- 0 ---- +x
//         /|
//        / |
//     +z  -y
//
// The origin is a loosely defined default reference position, e.g. used as the
// default home location (e.g. centered on the screen initially), etc.


//============================================================================
class World
{
public:
	//----------------------------------------------------------------------------
	// Cfg...
	//----------------------------------------------------------------------------
	enum class LoopMode : unsigned {
		Half_Matrix, // Better for Directionless/Undirected/Unordered interactions
		Full_Matrix, // Better for Directional/Ordered interactions

		Default = Half_Matrix,
		UseDefault = unsigned(-1), //!! Not actually part of the value set (but an add-on type!), but C++...
	};

protected:
	SimApp& app; //!! Rename to app_! But AFAICR `.app` is still used directly by derived classes!
	LoopMode loop_mode_; // Don't save: not a world property, just a processing option!
//!!	size_t entity_size_;

	// Called by app-engine bridge: App<Traits>::create_world() (NOT by the App<Traits> ctor, obviously).
	//!! Reconcile with init()!...
//!!	World(size_t app_entity_size) //!! Make it support different entity classes later!
//!!		: entity_size_(app_entity_size)
//!!	{}

	// No copy:
	World(const World&)             = delete;
	World& operator= (const World&) = delete;
	//!! But add move!...
	//!!...

public:
	World(SimApp& _app)
		: app(_app)
		, loop_mode_(LoopMode::Default)
	{}

	virtual void init() {}

	void update(TimeSpan dt);
	virtual void update_intrinsic(TimeSpan dt) { IGNORE dt; }
	virtual void update_pairwise (TimeSpan dt) { IGNORE dt; }
	virtual void update_global   (TimeSpan dt) { IGNORE dt; }

	virtual size_t entity_count() const = 0;
	auto     loop_mode() const     { return loop_mode_; }
	void set_loop_mode(LoopMode m) { loop_mode_ = m; }

	virtual ~World() = default;

	//----------------------------------------------------------------------------
	// "Standard" Hooks...
	//!!
	//!! Just an example! This is pretty ad-hoc, and shouldn't even be here, but
	//!! in the typed template layer!...
	//!!
	//----------------------------------------------------------------------------
	/*
	virtual bool collide_hook(World* w, OON::Model::Entity* obj1, OON::Model::Entity* obj2)
	{w, obj1, obj2;
		return false;
	}
	*/
//	virtual bool collide_hook(World* w, OON::Model::Entity* obj1, OON::Model::Entity* obj2, double distance);
//	virtual bool touch_hook(World* w, OON::Model::Entity* obj1, OON::Model::Entity* obj2);

	// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
	// `event` represents the physical property/condition that made it think these might interact.
	//!!NOTE: This will change to the objects themselves being notified (not the game "superclass")!
//	virtual void undirected_interaction_hook(World* w, OON::Model::Entity* obj1, OON::Model::Entity* obj2, float dt, double distance, ...);
//	virtual void directed_interaction_hook(World* w, OON::Model::Entity* source, OON::Model::Entity* target, float dt, double distance, ...);
	//----------------------------------------------------------------------------
	virtual void undirected_interaction_hook(World* w, Entity* obj1, Entity* obj2, float dt, double distance, ...)
	{IGNORE w, obj1, obj2, dt, distance;
	}

	virtual void directed_interaction_hook(World* w, Entity* source, Entity* target, float dt, double distance, ...)
	{IGNORE w, source, target, dt, distance;
	}

	virtual bool collide_hook(World* w, Entity* obj1, Entity* obj2, double distance)
	{IGNORE w, obj1, obj2, distance;
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		//!!...body->p -= ds...;
		return false;
	}

	virtual bool touch_hook(World* w, Entity* obj1, Entity* obj2)
	{IGNORE w, obj1, obj2;
		return false;
	}

	/*!!UPDATE/DELETE/MOVE NOTE:
	// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
	//!!The model should also pass the physical property/condition ("event type") that made it think these may interact!
	//!!A self-documenting alternative would be calling a matching function for each known such event,
	//!!but that might be prohibitively expensive in that tight loop, especiall if most callbacks would
	//!!just do nothing.
	//!!NOTE: This will anyway change to the objects themselves being notified (not the game "superclass")!
	void SimApp::interaction_hook(Model::World* w, Model::World::Event event, Entity* obj1, Entity* obj2, ...)
	{w, event, obj1, obj2;
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
	}
	!!*/


}; // class World


} // namespace Model::Core
} // namespace Szim


#endif // _AMWDMC$SDFY7JM8FM4F60YUCMFV7NH489TYU_