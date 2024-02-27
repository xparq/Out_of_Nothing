#ifndef _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_
#define _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_

#include "Engine/Config.hpp"
#include "Engine/Model.hpp" // Includes Physics.hpp, which includes Math.hpp
#include "Math/Vector2.hpp"
#include "Object.hpp" //!!This just includes World.hpp back, intentionally! :)
                      //!!(Wouldn't be that way if World::Body{} could just be defined there, separetely.)

#include <memory>     // shared_ptr
#include <vector>
//!!No, not yet. It's just too cumbersome, for too little gain:
//!!#include <optional> // for load()

/*!!
namespace std {
	class istream;
	class ostream;
}
MSVC accepted fw-declaring them, but only inside the classes (below)!
And it actually crashed the MSVC compiler, when I moved them here!... :-o (-> #195)
GCC was also really freaking out from those declarations...
AND CRASHED TOO!!! :)) (Albeit in Audio.cpp, probably unrelated; 1/2 hour before MSVC)! :D (-> #195)
!!*/
//!!OK, fuck it...:
#include <iostream>

namespace Szim { class SimApp; } //! Sigh, must predeclare it here, outside the namespace...

namespace Model {


// World coordinate system (Right-handed, like OpenGL):
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


static constexpr char const* VERSION = "0.1.3";

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

	enum Event { None, Interacting, Collided, Terminated };

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

	struct Body //!! : public Serializable //! No: this would kill the C++ designated init syntax! :-/
	                                       //! Also old-school; template-/concept-based approaches are superior.
	                                       //! Keep it trivially_copyable (not POD: they can't have ctors!) for easy loading!
	//! Inner class of World, because it depends on the physics (e.g. constants).
	//!! But that could be addressed by just being in the same namespace,
	//!! and then stuff that only need Entity could spare including the World.
	{
		//!!ObjConfig cfg; // basically the obj. type
		constexpr static auto Unlimited = Model::Unlimited;

		struct {
			bool gravity_immunity = false;
			bool free_color = false; // T doesn't affect color
		} superpower;

		// Presets:
		NumType lifetime = Unlimited; // how many s to Event::Decay; < 0 means stable end state that can't decay (any further)
		NumType r = 0; // Calculated from mass and density
		NumType density = Phys::DENSITY_ROCK / 2; //!!low-density objects should look like Swiss cheese! ;)
		Math::Vector2<NumType> p{0, 0};
		Math::Vector2<NumType> v{0, 0};
		float T = 0; // affected by various events; represented by color

		// Preset/recomputed:
		uint32_t color = 0; // if left 0, it'll be recalculated from T (if not 0)
			// RGB (Not containing an alpha byte (at LSB), so NOT compatible with the SFML Color ctors!
			// The reason is easier add_body() calls here.)

		NumType mass;

		//!! Ugly hack to start generalizing object compositions & to allow the world
		//!! to calc. propulsion without consulting the controller. (I mean this is still
		//!! less ugly than these being part of the game app (controller) itself! :) )
		//!! Should be handled later indirectly (and uniformly) via object configurations
		//!! (which would basically be a flexible type system).
		//!!
		//!!BTW, thrust should be axial anyway, so these 4 should be just 2:
		//!!ALSO: REPLACE WITH A GENERIC (dynamically built) Structure COMPONENT + ("OPTONAL") TYPE INFO!
		//!!      ("OPTIONAL" 'coz the structure itself *IS* the type info, it's just cumbersome to work with!)
		Thruster thrust_up    { Math::MyNaN<NumType> }; // Gets repalced by "real" numbers for objects with actually functioning thrusters.
		Thruster thrust_down  { Math::MyNaN<NumType> };
		Thruster thrust_left  { Math::MyNaN<NumType> };
		Thruster thrust_right { Math::MyNaN<NumType> };

		//! Alas, can't do this with designated inits: Body() : mass(powf(r, 3) * density) {} :-(
		//! So... (see e.g. add_body()):
		void recalc();
		bool can_expire() const noexcept { return lifetime > 0; }
		void terminate()  noexcept { lifetime = 0; } // Currently the best fit...
		bool terminated() const noexcept { return lifetime == 0; }
		void on_event(Event e, ...); //! Alas, can't be virtual: that would kill the C++ init. list syntax! :-o :-/

		// Ops.:
		bool has_thruster() { return thrust_up.thrust_level() != Math::MyNaN<float>; } //!! Ugh!... :-o :)
		void add_thrusters() { // Umm...: ;)
			thrust_up.thrust_level(0);
			thrust_down.thrust_level(0);
			thrust_left.thrust_level(0);
			thrust_right.thrust_level(0);
		}
		bool is_player() { return has_thruster(); } //!! ;)

//!!		class std::ostream; class std::istream;
		bool        save(std::ostream&);
		static bool load(std::istream&, World::Body* result = nullptr); // Verifies only (comparing to *this) if null
	};

	//------------------------------------------------------------------------
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
	size_t add_body(Body const& obj);
	size_t add_body(Body&& obj);
	void remove_body(size_t ndx);

	bool is_colliding([[maybe_unused]] const Body* obj1, [[maybe_unused]] const Body* obj2)
	// Takes the body shape into account.
	// Note: a real coll. calc. (e.g. bounding box intersect.) may not need to the distance to be calculated.
	{
		//auto distance = sqrt(pow(globe->p.x - body->p.x, 2) + pow(globe->p.y - body->p.y, 2));
		return false;
	}

	bool is_colliding(const Body* obj1, const Body* obj2, NumType distance)
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

	std::vector< std::shared_ptr<Body> > bodies; //!! Can't just be made `atomic` by magic... (wouldn't even compile)

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


//------------------------------------------------------------------------
using Entity = World::Body;


} // namespace Model

#endif // _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_