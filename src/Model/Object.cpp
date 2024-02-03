#include "Object.hpp"
#include <cassert>
//#include <iostream>
//	using std::cerr, std::endl;


namespace Model {

//!!?? Put an EventSubscriber* pointer here to the global OON_* instance to help propagating events?!
//!!   ...That would require an EventSubscriber interface first, also then actually used by OON. ;)


//============================================================================
void World::Body::recalc()
{
//	mass = powf(r, 3) * density;
	r = Phys::radius_from_mass_and_density(mass, density);
//	Phys::BV_to_T_and_RGB(Phys::T_to_BV(T), superpower.free_color ? nullptr : &color);
	Phys::T_to_RGB_and_BV(T, superpower.free_color ? nullptr : &color);
}

//----------------------------------------------------------------------------
void World::Body::on_event(Event e, ...)
{
	r *= 0.1f; //!!Just to see if it works at all!...
}

} // namespace Model
