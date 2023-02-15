#include "OON.hpp"

void OON::up_thruster_start()    { world.bodies[globe_ndx]->thrust_up.thrust_level(CFG_THRUST_FORCE); }
void OON::down_thruster_start()  { world.bodies[globe_ndx]->thrust_down.thrust_level(CFG_THRUST_FORCE); }
void OON::left_thruster_start()  { world.bodies[globe_ndx]->thrust_left.thrust_level(CFG_THRUST_FORCE); }
void OON::right_thruster_start() { world.bodies[globe_ndx]->thrust_right.thrust_level(CFG_THRUST_FORCE); }
void OON::up_thruster_stop()     { world.bodies[globe_ndx]->thrust_up.thrust_level(0); }
void OON::down_thruster_stop()   { world.bodies[globe_ndx]->thrust_down.thrust_level(0); }
void OON::left_thruster_stop()   { world.bodies[globe_ndx]->thrust_left.thrust_level(0); }
void OON::right_thruster_stop()  { world.bodies[globe_ndx]->thrust_right.thrust_level(0); }



void OON::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
//		if (!obj1->is_player())
//			obj1->color += 0x3363c3;
}
