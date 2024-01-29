#ifndef _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_
#define _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_

#include "Engine/Config.hpp"

#include "Engine/Model.hpp" //!! Just a placeholder, plus for the kludge symbol Model::Unlimited

#include "Physics.hpp"
#include "Math.hpp"   // Included by "Physics.hpp", though...
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

namespace Szim {
class SimApp; //! Sigh, must predeclare it here, outside the namespace...
              //! Curiously, it's not in the "global" :: namespece, so ::SimApp;
              //! wouldn't work from within the Model! :-o
}

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


static constexpr char const* VERSION = "0.1.2";

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
	enum Event { None, Collided, Terminated };

	enum GravityMode : unsigned { Off, Normal, Skewed }; //!! #405

	struct Body //!! : public Serializable //! No: this would kill the C++ init list syntax!...
	                                       //! So, just keep it a memcpy-able POD type for easy loading!
	//! Inner class of World, because it depends on the physics (e.g. constants).
	//!! But that could be addressed by just being in the same namespace,
	//!! and then stuff that only need Entity could spare including the World.
	{
		//!!ObjConfig cfg; // basically the obj. type
		constexpr const static auto Unlimited = Model::Unlimited;

		struct {
			bool gravity_immunity = false;
			bool free_color = false; // T doesn't affect color
		} superpower;

		// Presets:
		float lifetime = Unlimited; // how many s to Event::Decay; < 0 means stable end state that can't decay (any further)
		float r = 0; // Calculated from mass and density
		float density = Physics::DENSITY_ROCK / 2; //!!low-density objects should look like Swiss cheese! ;)
		Math::Vector2f p{0, 0};
		Math::Vector2f v{0, 0};
		float T = 0; // affected by various events; represented by color

		// Preset/recomputed:
		uint32_t color = 0; // if left 0, it'll be recalculated from T (if not 0)
			// RGB (Not containing an alpha byte (at LSB), so NOT compatible with the SFML Color ctors!
			// The reason is easier add_body() calls here.)

		float mass;

		//!! Ugly hack to start generalizing object compositions & to allow the world
		//!! to calc. propulsion without consulting the controller. (I mean this is still
		//!! less ugly than these being part of the game app (controller) itself! :) )
		//!! Should be handled later indirectly (and uniformly) via object configurations
		//!! (which would basically be a flexible type system).
		//!!
		//!!BTW, thrust should be axial anyway, so these 4 should be just 2:
		//!!ALSO: REPLACE WITH A GENERIC (dynamically built) Structure COMPONENT + ("OPTONAL") TYPE INFO!
		//!!      ("OPTIONAL" 'coz the structure itself *IS* the type info, it's just cumbersome to work with!)
		Thruster thrust_up    { Math::MyNaN }; // Gets repalced by "real" numbers for objects with actually functioning thrusters.
		Thruster thrust_down  { Math::MyNaN };
		Thruster thrust_left  { Math::MyNaN };
		Thruster thrust_right { Math::MyNaN };

		//! Alas, can't do this with designated inits: Body() : mass(powf(r, 3) * density) {} :-(
		//! So... (see e.g. add_body()):
		void recalc();
		bool can_expire() const noexcept { return lifetime > 0; }
		void terminate()  noexcept { lifetime = 0; } // Currently the best fit...
		bool terminated() const noexcept { return lifetime == 0; }
		void on_event(Event e, ...); //! Alas, can't be virtual: that would kill the C++ init. list syntax! :-o :-/

		// Ops.:
		bool has_thruster() { return thrust_up.thrust_level() != Math::MyNaN; } //!! Ugh!... :-o :)
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
	void update(float dt, Szim::SimApp& app); // ++world

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

	bool is_colliding(const Body* obj1, const Body* obj2, float distance)
	// Only for circles yet!
	{
		return distance <= obj1->r + obj2->r;
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
	float friction = 0.03f; //!!Take its default from the cfg!
	bool  _interact_all = false; // Bodies react to each other too, or only the player(s)?
	GravityMode gravity_mode;   //! v0.1.0
	float gravity = Physics::G; //! v0.1.1

	std::vector< std::shared_ptr<Body> > bodies; //! alas, can't just be made "atomic" by magic... (won't even compile)

//!!	class std::ostream; class std::istream;
	bool        save(std::ostream& out, const char* version = nullptr);
	static bool load(std::istream& in, World* result = nullptr); // Verifies only (comparing to *this) if null
//!! std::optional<World> load(std::istream& in);

//------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	World();
	World(const World& other) { _copy(other); }
	World& operator= (const World& other)  { _copy(other); return *this; }
	//!!Say sg. about move, too! I guess they are inhibited by the above now.

	void _copy(World const& other);
}; // class World


//------------------------------------------------------------------------
using Entity = World::Body;


} // namespace Model

#endif // _795460BVY2TNGHM02458NV7B6Y0WNCM2456Y_