#include "OON.hpp"

#include <cstdlib>
	using std::rand; // and the RAND_MAX macro!
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>

using namespace Model;
//using namespace UI;
using namespace std;


//----------------------------------------------------------------------------
bool OON::poll_and_process_controls()
{
	bool action = false;
	if (_ctrl_update_thrusters()) {
		action = true;
//		if (kbd_state[KBD_STATE::SPACE]) {
//			action = true;
//			exhaust_burst(5);
//		}
	}
	// Allow this now irrespective of any engine firing:
	if (kbd_state[KBD_STATE::SPACE]) {
		action = true;
		exhaust_burst(5);
	}
	return action;
}

//----------------------------------------------------------------------------
void OON::up_thruster_start()    { world.bodies[globe_ndx]->thrust_up.thrust_level(CFG_THRUST_FORCE); }
void OON::down_thruster_start()  { world.bodies[globe_ndx]->thrust_down.thrust_level(CFG_THRUST_FORCE); }
void OON::left_thruster_start()  { world.bodies[globe_ndx]->thrust_left.thrust_level(CFG_THRUST_FORCE); }
void OON::right_thruster_start() { world.bodies[globe_ndx]->thrust_right.thrust_level(CFG_THRUST_FORCE); }
void OON::up_thruster_stop()     { world.bodies[globe_ndx]->thrust_up.thrust_level(0); }
void OON::down_thruster_stop()   { world.bodies[globe_ndx]->thrust_down.thrust_level(0); }
void OON::left_thruster_stop()   { world.bodies[globe_ndx]->thrust_left.thrust_level(0); }
void OON::right_thruster_stop()  { world.bodies[globe_ndx]->thrust_right.thrust_level(0); }

bool OON::_ctrl_update_thrusters()
{
	auto drv = false;
	if (/*kbd_state[KBD_STATE::UP]    || */kbd_state[KBD_STATE::W]) { drv = true; up_thruster_start(); }    else up_thruster_stop();
	if (/*kbd_state[KBD_STATE::DOWN]  || */kbd_state[KBD_STATE::S]) { drv = true; down_thruster_start(); }  else down_thruster_stop();
	if (/*kbd_state[KBD_STATE::LEFT]  || */kbd_state[KBD_STATE::A]) { drv = true; left_thruster_start(); }  else left_thruster_stop();
	if (/*kbd_state[KBD_STATE::RIGHT] || */kbd_state[KBD_STATE::D]) { drv = true; right_thruster_start(); } else right_thruster_stop();
	return drv;
}

/*
bool OON::_ctrl_driving()
{
	return ((kbd_state[KBD_STATE::UP] || kbd_state[KBD_STATE::DOWN] || kbd_state[KBD_STATE::LEFT] || kbd_state[KBD_STATE::RIGHT] || 
				kbd_state[KBD_STATE::W] || kbd_state[KBD_STATE::S] || kbd_state[KBD_STATE::A] || kbd_state[KBD_STATE::D]);
}
*/

//----------------------------------------------------------------------------
void OON::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
//		if (!obj1->is_player())
//			obj1->color += 0x3363c3;
}


//----------------------------------------------------------------------------
void OON::spawn(size_t n)
{
	add_bodies(n);
}

//----------------------------------------------------------------------------
void OON::exhaust_burst(size_t n/* = 50*/)
{
/*!! Too boring with all these small particles:
	auto constexpr r_min = world.CFG_GLOBE_RADIUS / 10;
	auto constexpr r_max = world.CFG_GLOBE_RADIUS * 0.3;
	auto constexpr p_range = world.CFG_GLOBE_RADIUS * 2;
	auto constexpr v_range = world.CFG_GLOBE_RADIUS * 3; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
*/
	auto constexpr r_min = world.CFG_GLOBE_RADIUS / 10;
	auto constexpr r_max = world.CFG_GLOBE_RADIUS * 0.5;
	auto constexpr p_range = world.CFG_GLOBE_RADIUS * 5;
	auto constexpr v_range = world.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
	for (int i = 0; i++ < n;) {
		add_body({
			.r = (float) ((rand() * (r_max - r_min)) / RAND_MAX ) //! suppress warning "conversion from double to float..."
					+ r_min,
			.p = { (rand() * p_range) / RAND_MAX - p_range/2 + world.bodies[globe_ndx]->p.x - world.bodies[globe_ndx]->v.x * 0.1f,   //!!...jesus, that literal dt here! :-o
			       (rand() * p_range) / RAND_MAX - p_range/2 + world.bodies[globe_ndx]->p.y - world.bodies[globe_ndx]->v.y * 0.1f }, //!!...jesus, that literal dt here! :-o
			.v = { (rand() * v_range) / RAND_MAX - v_range/2 + world.bodies[globe_ndx]->v.x * 0.1f,
			       (rand() * v_range) / RAND_MAX - v_range/2 + world.bodies[globe_ndx]->v.y * 0.1f },
			.color = (uint32_t) (float)0xffffff * rand(),
		});
	}
}


//----------------------------------------------------------------------------
size_t OON::add_body()
{
	auto constexpr r_min = world.CFG_GLOBE_RADIUS / 9;
	auto constexpr r_max = world.CFG_GLOBE_RADIUS * 3;
	auto constexpr p_range = world.CFG_GLOBE_RADIUS * 30;
	auto constexpr v_range = world.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
//cerr << "Adding new object #" << world.bodies.size() + 1 << "...\n";
	return add_body({
		.r = (float) (((float)rand() * (r_max - r_min)) / RAND_MAX ) //! suppress warning "conversion from double to float..."
				+ r_min,
		.p = { (rand() * p_range) / RAND_MAX - p_range/2 + world.bodies[globe_ndx]->p.x,
		       (rand() * p_range) / RAND_MAX - p_range/2 + world.bodies[globe_ndx]->p.y },
		.v = { (rand() * v_range) / RAND_MAX - v_range/2 + world.bodies[globe_ndx]->v.x * 0.05f,
		       (rand() * v_range) / RAND_MAX - v_range/2 + world.bodies[globe_ndx]->v.y * 0.05f },
		.color = 0xffffff & ((uint32_t) rand() * rand()),
	});
}

//----------------------------------------------------------------------------
void OON::remove_body()
{
	if (world.bodies.size() < 2) { // Leave the "globe" (so not ".empty()")!
cerr <<	"No more \"free\" items to delete.\n";
		return;
	}

	auto ndx = 1/*leave the globe!*/ + rand() * ((world.bodies.size()-1) / (RAND_MAX + 1));
//cerr << "Deleting object #"	 << ndx << "...\n";
	assert(ndx > 0);
	assert(ndx < world.bodies.size());
	remove_body(ndx);
}

//----------------------------------------------------------------------------
void OON::add_bodies(size_t n)
{
	while (n--) add_body();
}

//----------------------------------------------------------------------------
void OON::remove_bodies(size_t n/* = -1*/)
{
	if (n == (unsigned)-1) n = world.bodies.size();
	while (n--) remove_body();
}