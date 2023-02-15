#ifndef __WORLD_SFML__
#define __WORLD_SFML__

#include "Maths.hpp"
#include "Physics.hpp"
#include "Object.hpp" //!!This just includes World.hpp back, intentionally! :)
                      //!!(Wouldn't be that way if World::Body{} could just be defined there, separetely.)

#include "SFML/Vector2.hpp"
	//! NOTE: despite the filename, it's NOT being included from SFML, but has
	//! been ripped out of it and copied here, because it's nicely self-containing,
	//! so we can keep using it, while still detaching from the *entire* lib itself!
	//! To avoid duplicate definitions and confusiion, the local version has changed
	//! the `sf` namespace from to `sfml`!)
	//! The actual types are of course syntactically incompatible though, so some
	//! awkward manual (compile-time) fiddling might still be required occasionally.

#include <memory> // shared_ptr
#include <vector>

class SimApp; //! Sigh, must predeclare it here, outside the namespace...
              //! Curiously, it's not in the "global" :: namespece either
              //! by default, so ::SimApp; wouldn't work there either! :-o
namespace Model {

//----------------------------------------------------------------------------
class World
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
	struct Body
	//! Inner class of World, because it depends on the physics (e.g. constants).
	{
		//!!ObjConfig cfg; // basically the obj. type
		struct {
			bool gravity_immunity = false;
			bool free_color = false; // T doesn't affect color
		} superpower;

		// Preset:
		float r = 0;
		float density{Physics::DENSITY_ROCK / 2}; //!!low-density objects should look like Swiss cheese! ;)
		sfml::Vector2f p{0, 0};
		sfml::Vector2f v{0, 0};
		float T = 0; // affected by various events; represented by color
		float lifetime = 300; // 5 min...
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

		// Ops.:
		bool has_thrusters() { return thrust_up.thrust_level() != MyNaN; }
		void add_thrusters() { // Umm...: ;)
			thrust_up.thrust_level(0);
			thrust_down.thrust_level(0);
			thrust_left.thrust_level(0);
			thrust_right.thrust_level(0);
		}
		bool is_player() { return has_thrusters(); } // ;)
	};

	//------------------------------------------------------------------------
	void recalc_next_state(float dt, SimApp& app); // ++world

	enum Event { None, Collision };

//----------------------------------------------------------------------------
// API Ops...
//----------------------------------------------------------------------------
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

	std::vector< std::shared_ptr<Body> > bodies; //! alas, can't just be made "atomic" by magic... (won't even compile)

	float FRICTION = 0.03f;

	bool _interact_all = false; // bodies react with each other too, or only with the player(s)


//------------------------------------------------------------------------
// Housekeeping...
//----------------------------------------------------------------------------
public:
	World() = default;
	World(const World& other) = default;
	World& operator= (const World& other) = default;
};

} // namespace
#endif // __WORLD_SFML__