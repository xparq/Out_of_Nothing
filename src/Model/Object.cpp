#include "Object.hpp"

namespace Model {

//============================================================================
void World::Body::recalc()
{
	mass = powf(r, 3) * density;
	Physics::BV_to_T_and_RGB(Physics::T_to_BV(T), superpower.free_color ? nullptr: &color);
}

}; // namespace
