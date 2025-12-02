#ifndef _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_
#define _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_

#include "app/model/vocab.hpp"
#include "Physics.hpp"
#include "app/model/Entity.hpp"

#include "Szim/World.hpp" // Szim::World<Cfg>

#include <memory>     // shared_ptr
#include <vector>
//!!No, not yet; just too cumbersome for too little gain:
//!!#include <optional> // for load()
#include <iosfwd> // for save/load


namespace Szim { class SimApp; } //! Sigh, must predeclare it here, outside the namespace...

namespace OON::Model {

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


static constexpr char const* VERSION = "0.1.5";

//============================================================================
	//----------------------------------------------------------------------------
	// API Types...
	//----------------------------------------------------------------------------
	enum class GravityMode : unsigned {
		Off = 0,
		Hyperbolic,
		Realistic,
		Experimental,

		Default = Hyperbolic,
		UseDefault = unsigned(-1), //!! Not actually part of the value set (but an add-on type!), but C++...
	}; //!! #405, #65, #361

	using NumType = Phys::NumType;

	enum class CollisionMode : unsigned {
		Off = 0, // Do nothing, ignore even the imminent div/0 errors...
		Glide_Through = 1, // Slide over each other at constant speed
	};

	struct Properties {
		//!! REVISE save/load, WHENEVER CHANGING THESE!
		float         friction;
		GravityMode   gravity_mode;  //! v0.1.0
		Phys::NumType gravity;       //! v0.1.1 //!!Take its default from the cfg instead!
		bool          interact_n2n;  // Entities react to each other, or only the player(s)?
		double        repulsion_stiffness; //! 0.1.5 (trying to use double by default now...)
		CollisionMode collision_mode; //! 0.1.5 no gravity inside a body

		static const Properties defaults; // See it defined in the .cpp!
	};

	//--------------------------------------------------------------------
	struct WorldConfig
	{
		using        PropsT  = Properties;
		using        EntityT = Entity;
	};
	//--------------------------------------------------------------------


//----------------------------------------------------------------------------
class World : public Szim::Model::World<WorldConfig>
//!! Should be split into an abstract base in the Engine to facilitate universal
//!! features like interactions etc., and the detailed, specific one of the app!
{
//----------------------------------------------------------------------------
// API Config... (!!Parts being migrated from the "Controller"...)
//               (!!-> #110: Actually load from the real config!!)
//----------------------------------------------------------------------------
public:
	static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m

//----------------------------------------------------------------------------
// API Types...
//----------------------------------------------------------------------------
public:

/*!! Moved to Szim::Model::Core::World
	enum class LoopMode : unsigned {
		Half_Matrix, // Better for Directionless/Undirected/Unordered interactions
		Full_Matrix, // Better for Directional/Ordered interactions

		Default = Half_Matrix,
		UseDefault = unsigned(-1), //!! Not actually part of the value set (but an add-on type!), but C++...
	};
!!*/

//----------------------------------------------------------------------------
// Setup...
//----------------------------------------------------------------------------

	// Reuse the ctors — most notably the mandatory `World(App)` one!
	using Szim::Model::World<WorldConfig>::World;

	void init() override;

	// add_body() will do an initial recalc() on the newly added entity,
	// mainly to allow partially initialized template objects as inputs:
	EntityID add_body(Entity const& obj);
	EntityID add_body(Entity&& obj);
	void remove_body(EntityID ndx);

//----------------------------------------------------------------------------
// Modeling...
//----------------------------------------------------------------------------
	void update_intrinsic(DeltaT dt) override;
	void update_pairwise (DeltaT dt) override;
	void update_global   (DeltaT dt) override;

	bool is_colliding(const Szim::Model::Core::Entity* obj1, const Szim::Model::Core::Entity* obj2) const;
	bool is_colliding(const Szim::Model::Core::Entity* obj1, const Szim::Model::Core::Entity* obj2, Phys::Length distance) const;


	//------------------------------------------------------------------------
	// Model callback implementations...
	//!! Move these out of the direct app code to an app-level custom model class set!
	//!! And then the model callback mechanism could be simplified to not doing it in
	//!! the core abstract Model at all, but in the custom layer, only when needed.
	void undirected_interaction_hook(Szim::Model::Core::World* w, Szim::Model::Core::Entity* obj1, Szim::Model::Core::Entity* obj2, float dt, double distance, ...) override;
	void directed_interaction_hook(Szim::Model::Core::World* w, Szim::Model::Core::Entity* source, Szim::Model::Core::Entity* target, float dt, double distance, ...) override;
	bool touch_hook(Szim::Model::Core::World* w, Szim::Model::Core::Entity* obj1, Szim::Model::Core::Entity* obj2) override;

//----------------------------------------------------------------------------
// Internals (C++ mechanics, persistence etc.)...
//----------------------------------------------------------------------------
public:
/*!!OLD, before the app had a World ptr:
	World(const World& other) { _copy(other); }
	World& operator= (const World& other)  { _copy(other); return *this; }
	//!!Say sg. about move, too! I guess they are inhibited by the above now.

	void _copy(World const& other);
!!*/
	bool        save(std::ostream& out, const char* version = nullptr) const;
	static bool load(std::istream& in, World* result = nullptr); //!!NOT YET: null means "Verify only" (comparing to *this)
	//!!??static std::optional<World> load(std::istream& in);

}; // class World


} // namespace OON::Model

#endif // _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_