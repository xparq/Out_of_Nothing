#include "OON.hpp"
#include "misc/sign.hpp"

#include <cstdlib>
	using std::rand; // and the RAND_MAX macro!
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>

using namespace Model;
using namespace Math;
using namespace UI;
using namespace std;

//----------------------------------------------------------------------------
bool OON::poll_and_process_controls()
{
	bool action = false;
	if (_ctrl_update_thrusters()) {
		action = true;
//		if (keystate(SPACE)) {
//			action = true;
//			exhaust_burst(5);
//		}
	}
	// Allow this now irrespective of any engine firing:
	if (keystate(SPACE)) {
		action = true;
		exhaust_burst(5);
	}

	if (_ctrl_update_pan())
		action = true;

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
	if (keystate(UP))    { drv = true;    up_thruster_start(); } else    up_thruster_stop();
	if (keystate(DOWN))  { drv = true;  down_thruster_start(); } else  down_thruster_stop();
	if (keystate(LEFT))  { drv = true;  left_thruster_start(); } else  left_thruster_stop();
	if (keystate(RIGHT)) { drv = true; right_thruster_start(); } else right_thruster_stop();
	return drv;
}

//----------------------------------------------------------------------------
void OON::pan_reset()  { view.offset = {0, 0}; }
void OON::pan(Vector2f delta) { pan_x(delta.x); pan_y(delta.y); }
void OON::pan_x(float delta) { view.offset.x += delta; }
void OON::pan_y(float delta) { view.offset.y += delta; }
void OON::pan_to_entity(size_t id)
{
	const auto& body = world.bodies[id];
	view.offset = - body->p * view.zoom;
}

void OON::zoom(float factor)
{
//!!pre_zoom_hook(factor);
	view.zoom *= factor;
	post_zoom_hook(factor);
}


bool OON::_ctrl_update_pan()
{
	auto action = false;

	if (keystate(W)) { action = true; pan_step_y =  CFG_PAN_STEP; }
	if (keystate(S)) { action = true; pan_step_y = -CFG_PAN_STEP; }
	if (keystate(A)) { action = true; pan_step_x =  CFG_PAN_STEP; }
	if (keystate(D)) { action = true; pan_step_x = -CFG_PAN_STEP; }

	if (!action) {
		if (pan_step_x) pan_step_x -= misc::sign(pan_step_x);
		if (pan_step_y) pan_step_y -= misc::sign(pan_step_y);
	}
	if (pan_step_x) pan_x(pan_step_x);
	if (pan_step_y) pan_y(pan_step_y);

	return action;
}

/*
bool OON::_ctrl_driving()
{
	return ((keystate(UP) || keystate(UI::DOWN) || keystate(LEFT) || keystate(RIGHT) ||
			 keystate(W) || keystate(UI::S) || keystate(A) || keystate(D));
}
*/

//----------------------------------------------------------------------------
void OON::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
//		if (!obj1->is_player())
//			obj1->color += 0x3363c3;
}


//----------------------------------------------------------------------------
size_t OON::add_body(World::Body&& obj)
{
	return world.add_body(std::forward<decltype(obj)>(obj));
}

//----------------------------------------------------------------------------
void OON::remove_body(size_t ndx)
{
	world.remove_body(ndx);
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


//----------------------------------------------------------------------------
void OON::spawn(size_t n, size_t parent_ndx)
//!!Should gradually become a method of the obect itself actually!
{
	assert(parent_ndx == globe_ndx); //!!invalid for multiplayer

	// -> #41: Enable inheritance
	auto const& parent = *(world.bodies[parent_ndx]);
	for (size_t i = 0; i < n; ++i) {
		auto ndx = add_body();
		auto& newborn = *(world.bodies[ndx]);
		newborn.T = parent.T; // #155: Inherit T
	}
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
