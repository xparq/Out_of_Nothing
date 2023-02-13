#ifndef __WORLD_SFML__
#define __WORLD_SFML__

#include <SFML/System/Vector2.hpp>

#include <cmath> // sqrt
#include <memory>
	// using std::shared_ptr;
#include <vector>

//namespace Model {

//!!Put these into some generic geometry helper thing:
template <typename T> T distance_2d(T dx, T dy)  { return sqrt(dx*dx + dy*dy); }
template <typename T> T distance_2d(T x1, T y1, T x2, T y2)
{
	T dx = x2 - x1, dy = y2 - y1;
	return distance_2d(dx*dx, dy*dy);
}


//----------------------------------------------------------------------------
class World // "Model"
//----------------------------------------------------------------------------
{

public:
static constexpr float MyNaN = 2e31f; // to avoid the pain of using the std NAN...
struct Physics
{
//	...
};
// Physics constants -- !!MOVE INTO Physics! --:
	//! `const` can't do non-integral statics! :-/
	static constexpr float G = 6.673e-11f; //!! No point keeping this real and all the others stretched,
	               //!! const unless a real orbital simulation is the goal (which isn't)!...
	static constexpr float DENSITY_ROCK = 2000.0f; // kg/m3

	float FRICTION = 0.03f;

// Internal state: -- !!NOT YET! SOME PRETTY EXTERNAL/PUBLIC STUFF, TOO!...
	bool _interact_all = false; // bodies react with each other too, or only with the player(s)

public:
	//------------------------------------------------------------------------
	struct Thruster {
		float _thrust_level = 0;
		float thrust_level(float new_thrust_level)
		{
			auto prev_thrust_level = _thrust_level;
			_thrust_level = new_thrust_level;
			return prev_thrust_level;
		}
		float thrust_level() const { return _thrust_level; }
	};
	//!!Thrusters should be vectorized, relative to the body orientation,
	//!!which is currently fixed to be identical to the world coordinate system...
	//------------------------------------------------------------------------
	struct Body
	//! Inner class of World, because it depends on the physics (e.g. constants).
	{
		//!!ObjConfig cfg; // basically the obj. type

		// preset:
		float r = 0;
		float density{World::DENSITY_ROCK / 2};

		// preset + updated:
		sf::Vector2f p{0, 0};
		sf::Vector2f v{0, 0};
		uint32_t color = 0; // RGB (Not containing an alpha byte (at LSB), so NOT compatible with the SFML Color ctors!
							// The reason is easier add_body() calls here.)

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

		struct {
			bool gravity_immunity = false;
		} superpower;

		// computed only:
		float mass;

		//! Can't do this to support designated inits: Body() : mass(powf(r, 3) * density) {}
		//! So... (see add_body()):
		void precalc() { mass = powf(r, 3) * density; }

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

public:
	enum Event { None, Collision };

public: // Just allow access for now...:
	std::vector< std::shared_ptr<Body> > bodies; //! alas, can't just be made "atomic" by magic... (won't even compile)

// Ops
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

};

class Engine_SFML;
//----------------------------------------------------------------------------
class World_SFML : public World
{
public:
	void recalc_next_state(float dt, Engine_SFML& engine); // ++world
};

//} // namespace
#endif // __WORLD_SFML__