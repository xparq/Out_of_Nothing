#ifndef __WORLD_SFML__
#define __WORLD_SFML__

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <cmath> // sqrt
#include <memory>
	// using std::shared_ptr;
#include <vector>


//!!Put these into some generic geometry helper thing:
template <typename T> T distance_2d(T dx, T dy)  { return sqrt(dx*dx + dy*dy); }
template <typename T> T distance_2d(T x1, T y1, T x2, T y2)
{
	T dx = x2 - x1, dy = y2 - y1;
	return distance_2d(dx*dx, dy*dy);
}


class Engine; // for callbacks
//----------------------------------------------------------------------------
class World // "Model"
//----------------------------------------------------------------------------
{

public:
//!!:) static constexpr float MyNaN = 2e31f; // to avoid the pain of using the std NAN...
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

// Internal state:
	float dt; // inter-frame increment of the world model time
	bool _paused = false;
	auto paused() { return _paused; }
	virtual void pause(bool state = true) = 0;

public:
struct Body
//! Inner class of World, because it depends on the physics (e.g. constants).
{
	// preset:
	float r = 0;
	float density{World::DENSITY_ROCK / 2};

	// preset + updated:
	sf::Vector2f p{0, 0};
	sf::Vector2f v{0, 0};
	uint32_t color = 0; // RGB (Not containing an alpha byte (at LSB), so NOT compatible with the SFML Color ctors!
	                    // The reason is easier add_body() calls here.)

	// computed only:
	float mass;

	//! Can't do this to support designated inits: Body() : mass(powf(r, 3) * density) {}
	//! So... (see add_body()):
	void precalc() { mass = powf(r, 3) * density; }
};

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
protected:
// Internal state:
	sf::Vector2f v = {0, 0};

public:

	void recalc_for_next_frame(Engine_SFML& engine); // ++world

	void pause(bool state = true) override
	{
		_paused = state;
		//! Need to start from 0 when unpausing.
		//! (The other reset, on pausing, is just redundant, for simplicity.)
		clock.restart();
	}

// Housekeeping
	sf::Clock clock;
};


#endif // __WORLD_SFML__

