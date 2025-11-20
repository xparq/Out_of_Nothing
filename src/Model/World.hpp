#ifndef _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_
#define _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_

#include "Szim/Metamodel.hpp"
#include "Physics.hpp"
#include "Entity.hpp"
#include "Szim/Config.hpp"

#include <memory>     // shared_ptr
#include <vector>
//!!No, not yet; just too cumbersome for too little gain:
//!!#include <optional> // for load()
#include <iosfwd> // for save/load


namespace Szim { class SimApp; } //! Sigh, must predeclare it here, outside the namespace...

namespace Model {


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
class World // The model world
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
	using NumType = Phys::NumType;

	enum class GravityMode : unsigned {
		Off = 0,
		Hyperbolic,
		Realistic,
		Experimental,

		Default = Hyperbolic,
		UseDefault = unsigned(-1), //!! Not actually part of the value set (but an add-on type!), but C++...
	}; //!! #405, #65, #361

	enum class CollisionMode : unsigned {
		Off = 0, // Do nothing, ignore even the imminent div/0 errors...
		Glide_Through = 1, // Slide over each other at constant speed
	};

	enum class LoopMode : unsigned {
		Half_Matrix, // Better for Directionless/Undirected/Unordered interactions
		Full_Matrix, // Better for Directional/Ordered interactions

		Default = Half_Matrix,
		UseDefault = unsigned(-1), //!! Not actually part of the value set (but an add-on type!), but C++...
	};

//----------------------------------------------------------------------------
// Setup...
//----------------------------------------------------------------------------

	void init(Szim::SimApp& app);

	// add_body() will do an initial recalc() on the newly added entity,
	// mainly to allow partially initialized template objects as inputs:
	EntityID add_body(Entity const& obj);
	EntityID add_body(Entity&& obj);
	void remove_body(EntityID ndx);

//----------------------------------------------------------------------------
// ++world...
//----------------------------------------------------------------------------
	void update(float dt, Szim::SimApp& app); //!!?? Can time remain just float?...

		void update_intrinsic(float dt, Szim::SimApp& app);
		void update_pairwise(float dt, Szim::SimApp& app);
		void update_global(float dt, Szim::SimApp& app);

	bool is_colliding(const Entity* obj1, const Entity* obj2) const;
	bool is_colliding(const Entity* obj1, const Entity* obj2, Phys::Length distance) const;

//----------------------------------------------------------------------------
// Data / World State... (!!-> Save/Load should cover this part (#76)!!)
//----------------------------------------------------------------------------
//protected: //!!NOT YET!
//!!Can't even declare these here (which would still be depressing...) in the current namespace:
//!!friend class OONApp;
//!!friend class OONApp_sfml;
//!!So just allow public access for now:
public:
	//!! REVISE _copy() WHENEVER CHANGING ANY OF THESE!

	struct Properties {
		//!! REVISE save/load, WHENEVER CHANGING THESE!
		float         friction;
		GravityMode   gravity_mode;  //! v0.1.0
		NumType       gravity;       //! v0.1.1 //!!Take its default from the cfg instead!
		bool          interact_n2n;  // Entities react to each other, or only the player(s)?
		double        repulsion_stiffness; //! 0.1.5 (trying to use double by default now...)
		CollisionMode collision_mode; //! 0.1.5 no gravity inside a body
	};
	static const Properties Default_Props;

	Properties props;

	std::vector< std::shared_ptr<Entity> > bodies; //!! Can't just be made `atomic` by magic... (wouldn't even compile)

	LoopMode loop_mode; // Don't save: not a world property, just a processing option!

//----------------------------------------------------------------------------
// Internals (C++ mechanics, persistence etc.)...
//----------------------------------------------------------------------------
public:
	World();
	World(const World& other) { _copy(other); }
	World& operator= (const World& other)  { _copy(other); return *this; }
	//!!Say sg. about move, too! I guess they are inhibited by the above now.

	void _copy(World const& other);

	bool        save(std::ostream& out, const char* version = nullptr);
	static bool load(std::istream& in, World* result = nullptr); //!!NOT YET: null means "Verify only" (comparing to *this)
	//!!??static std::optional<World> load(std::istream& in);

}; // class World


} // namespace Model

#endif // _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_