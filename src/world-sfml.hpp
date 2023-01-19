#ifndef __WORLD_SFML__
#define __WORLD_SFML__

#include "cfg.h"

#include <SFML/Graphics.hpp>

#include <cmath>
#include <memory> // shared_ptr
#include <vector>
#include <iostream> // cerr
using namespace std;


//----------------------------------------------------------------------------
class World // "Model"
//----------------------------------------------------------------------------
{
public:
struct Physics
{
//	...
};
// Physics constants -- !!MOVE INTO Physics! --:
	//! `const` can't do non-integral statics! :-/
	static constexpr float G = 6.673e-11; //!! No point keeping this real and all the others stretched,
	               //!! const unless a real orbital simulation is the goal (which isn't)!...
	static constexpr float DENSITY_ROCK = 2000; // kg/m3
	static constexpr float FRICTION = 0.3;

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
	vector< shared_ptr<Body> > bodies;

// Ops
	auto add_body(Body&& obj)
	{
		obj.precalc();
		bodies.push_back(make_shared<Body>(obj));

		return bodies.size() - 1;
	}

	bool is_colliding(const Body* obj1, const Body* obj2)
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

	auto collide(Body* obj1, Body* obj2)
	{
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->v = {0, 0}; // or bounce, or stick to the other body and take its v, or any other sort of interaction.
		//!!...body->p -= ds...;
	}

	auto notify(Event event, Body* obj1, Body* obj2, ...)
	{
		//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
		obj1->color += 0x0888c8;
	}
};

class Engine_SFML;
//----------------------------------------------------------------------------
class World_SFML : public World
{
protected:
// Internal state:
	float dt; // inter-frame increment of the world model time
	sf::Vector2f v = {0, 0};

public:

	void recalc_for_next_frame(const Engine_SFML& game); // ++world

// Housekeeping
	sf::Clock clock;
};


#endif // __WORLD_SFML__

