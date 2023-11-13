#ifndef __WORLD_SFML__
#define __WORLD_SFML__

#include "Physics.hpp" // #includes Math.hpp
#include "Math/Vector2.hpp"
//	using namespace Math;
	using Math::MyNaN;

#include "Object.hpp" //!!This just includes World.hpp back, intentionally! :)
                      //!!(Wouldn't be that way if World::Body{} could just be defined there, separetely.)

#include <memory> // shared_ptr
#include <vector>
//!!No, not yet. It's just too cumbersome, for too little gain:
//!!#include <optional> // for load()

namespace Szim {
class SimApp; //! Sigh, must predeclare it here, outside the namespace...
              //! Curiously, it's not in the "global" :: namespece, so ::SimApp;
              //! wouldn't work from within the Model! :-o
}
namespace Model {

static constexpr char const* VERSION = "0.0.1";

//============================================================================
class World // The model world
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
	enum Event { None, Collision, Decay };

	struct Body //!! : public Serializable //! No: this would kill the C++ init list syntax!...
	                                       //! So, just keep it a memcpy-able POD type for easy loading!
	//! Inner class of World, because it depends on the physics (e.g. constants).
	{
		//!!ObjConfig cfg; // basically the obj. type
		static constexpr float Unlimited = -1; //! Not an enum to avoid the `World::Body::Enumname::Unlimited` atrocity
		struct {
			bool gravity_immunity = false;
			bool free_color = false; // T doesn't affect color
		} superpower;

		// Presets:
		float lifetime = Unlimited; // how many s to Event::Decay; < 0 means stable end state that can't decay (any further)
		float r = 0;
		float density{Physics::DENSITY_ROCK / 2}; //!!low-density objects should look like Swiss cheese! ;)
		Math::Vector2f p{0, 0};
		Math::Vector2f v{0, 0};
		float T = 0; // affected by various events; represented by color

		// Preset/recomputed:
		uint32_t color = 0; // if left 0, it'll recalculated from T (if not 0)
			// RGB (Not containing an alpha byte (at LSB), so NOT compatible with the SFML Color ctors!
			// The reason is easier add_body() calls here.)

		// Always computed:
		float mass;

		//!! Ugly hack to start generalizing object compositions & to allow the world
		//!! to calc. propulsion without consulting the controller. (I mean this is still
		//!! less ugly than these being part of the game app (controller) itself! :) )
		//!! Should be handled later indirectly (and uniformly) via object configurations
		//!! (which would basically be a flexible type system).
		//!!
		//!!BTW, thrust should be axial anyway, so these 4 should be just 2:
		Thruster thrust_up{ MyNaN }; //! will be changed to "real" numbers for objects with actually functioning thrusters
		Thruster thrust_down{ MyNaN };
		Thruster thrust_left{ MyNaN };
		Thruster thrust_right {MyNaN };

		//! Alas, can't do this with designated inits: Body() : mass(powf(r, 3) * density) {} :-(
		//! So... (see e.g. add_body()):
		void recalc();
		bool can_decay() { return lifetime > 0; }
		void on_event(Event e, ...); //! Alas, can't be virtual: that would kill the C++ init. list syntax! :-o :-/

		// Ops.:
		bool has_thrusters() { return thrust_up.thrust_level() != MyNaN; }
		void add_thrusters() { // Umm...: ;)
			thrust_up.thrust_level(0);
			thrust_down.thrust_level(0);
			thrust_left.thrust_level(0);
			thrust_right.thrust_level(0);
		}
		bool is_player() { return has_thrusters(); } // ;)

		class std::ostream; class std::istream;
		bool        save(std::ostream&);
		static Body load(std::istream&);
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

	bool is_colliding(const Body* obj1, const Body* obj2)
	// Takes the body shape into account.
	// Note: a real coll. calc. (e.g. bounding box intersect.) may not need to the distance to be calculated.
	{obj1, obj2;
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
//!!Can't even declare these here (which would still be depressing!) from the middle of our current namespace:
//!!friend class OON;
//!!friend class OON_sfml;
//!!So just allow public access for now:
public:

	float FRICTION = 0.03f;
	bool _interact_all = false; // bodies react with each other too, or only with the player(s)

	std::vector< std::shared_ptr<Body> > bodies; //! alas, can't just be made "atomic" by magic... (won't even compile)

	class std::ostream; class std::istream;
	bool        save(std::ostream& out);
	static bool load(std::istream& in, World* result = nullptr); // Verifies only (comparing to *this) if null
//!! std::optional<World> load(std::istream& in);

//------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	World() = default;
	World(const World& other) { _clone(other); }
	World& operator= (const World& other)  { return _clone(other); }
	//!!Say sg. about move, too! I guess they are inhibited by the above now.

	World& _clone(World const& other);
}; // class World


} // namespace Model

#endif // __WORLD_SFML__