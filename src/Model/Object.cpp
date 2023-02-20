#include "Object.hpp"

namespace Model {

//!!?? Put a EventSubscriber* pointer here to the global OON_* instance to help propagating events?!
//!!   ...That would require an EventSubscriber interface first, also then actually used by OON. ;)


//============================================================================
void World::Body::recalc()
{
	mass = powf(r, 3) * density;
	Physics::BV_to_T_and_RGB(Physics::T_to_BV(T), superpower.free_color ? nullptr: &color);
}

//----------------------------------------------------------------------------
void World::Body::on_event(Event e, ...)
{
	r *= 0.1f; //!!Just to see if it works at all!...
}

}; // namespace
