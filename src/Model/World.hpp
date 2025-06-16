#ifndef _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_
#define _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_

#include "Engine/Metamodel.hpp"
#include "Physics.hpp"
#include "Entity.hpp"
#include "Engine/Config.hpp"

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
// Origin: center of the screen (window, view pane...)


static constexpr char const* VERSION = "0.1.4";

//============================================================================
class World // The model world
//!! Should be split into an abstract base in the Engine to facilitate universal
//!! features like interactions etc., and the detailed, specific one of the app!
{
//----------------------------------------------------------------------------
// API Config... (!!Parts being migrated from the "Controller"...)
//           (!!-> #110: Actually load from a real config!!)
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

	enum class LoopMode : unsigned {
		Half, // Better for Directionless/Undirected/Unordered interactions
		Full, // Better for Directed//Ordered interactions

//!! Not even a "loop mode" any more:
//!!		PlayerOnly, //!!?? Should this be an orthogonal bit-flag instead?
//!!		            //!!?? What EXACTLY should this even mean? (Should just do what _interact_all = false does?!)

		Default = Half,
		UseDefault = unsigned(-1), //!! Not actually part of the value set (but an add-on type!), but C++...
	};

	//--------------------------------------------------------------------
	// world init, ++world...
	//--------
	void init(Szim::SimApp& app);

	//!!?? Should time still be just float?...
	void update(float dt, Szim::SimApp& app);
	void update_before_interactions(float dt, Szim::SimApp& app);
	void update_pairwise_interactions(float dt, Szim::SimApp& app);
	void update_after_interactions(float dt, Szim::SimApp& app);

//----------------------------------------------------------------------------
// API Ops...
//----------------------------------------------------------------------------
	// add_body() also does an initial recalc() on the object, to allow
	// partially initialized template obj as input:
	size_t add_body(Entity const& obj);
	size_t add_body(Entity&& obj);
	void remove_body(size_t ndx);

	bool is_colliding([[maybe_unused]] const Entity* obj1, [[maybe_unused]] const Entity* obj2)
	// Takes the body shape into account.
	// Note: a real coll. calc. (e.g. bounding box intersect.) may not need to the distance to be calculated.
	{
		//auto distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));
		return false;
	}

	bool is_colliding(const Entity* obj1, const Entity* obj2, Phys::Length distance)
	// Only for circles yet!
	{
		return distance <= obj1->r + obj2->r; // false; // -> #526!
	}

//----------------------------------------------------------------------------
// Data / World State... (!!-> Save/Load should cover this part (#76)!!)
//----------------------------------------------------------------------------
//protected: //!!NOT YET!
//!!Can't even declare these here (which would still be depressing!) from the middle of the current namespace:
//!!friend class OONApp;
//!!friend class OONApp_sfml;
//!!So just allow public access for now:
public:
	//!! REVISE _copy(), and save/load, WHENEVER CHANGING THE DATA HERE!
	float friction = 0.03f; //!!Take its default from the cfg instead!
	GravityMode gravity_mode;   //! v0.1.0
	NumType gravity = Phys::G; //! v0.1.1 //!!Take its default from the cfg instead!
	bool  _interact_all = false; // Bodies react to each other too, or only the player(s)?
	                             //!! Reconcile with interaction_mode!

	std::vector< std::shared_ptr<Entity> > bodies; //!! Can't just be made `atomic` by magic... (wouldn't even compile)

	LoopMode loop_mode; // Not to be saved! (Not world state, but a processing option.)

//----------------------------------------------------------------------------
// Service functions (C++ mechanics, persistence etc.)...
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