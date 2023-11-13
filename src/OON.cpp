#include "OON.hpp"
#include "sz/sign.hh"

#include <cstdlib>
	using std::rand; // and the RAND_MAX macro!
#include <iostream>
	using std::cerr, std::endl;
#include <cassert>

using namespace Model;
using namespace Math;
using namespace UI;
using namespace std;

bool OON::init() // override
{
	const auto& w = const_world();
		//!! The model world has been implicitly created in SimApp yet!...

	// Add the "Player Superglobe" first
	//!! THIS MUST COME BEFORE CALLING add_bodies()!
	globe_ndx = add_player({.r = w.CFG_GLOBE_RADIUS, .density = Physics::DENSITY_ROCK, .p = {0,0}, .v = {0,0}, .color = 0xffff20});
	assert(entity_count() > globe_ndx);
	assert(player_entity_ndx() == globe_ndx);

	// Add 2 "moons" with fixed parameters (mainly for testing):
	add_body({.r = w.CFG_GLOBE_RADIUS/10, .p = {w.CFG_GLOBE_RADIUS * 2, 0}, .v = {0, -w.CFG_GLOBE_RADIUS * 2},
				.color = 0xff2020});
	add_body({.r = w.CFG_GLOBE_RADIUS/7,  .p = {-w.CFG_GLOBE_RADIUS * 1.6f, +w.CFG_GLOBE_RADIUS * 1.2f}, .v = {-w.CFG_GLOBE_RADIUS*1.8, -w.CFG_GLOBE_RADIUS*1.5},
				.color = 0x3060ff});

	// Game-related cmdline options...
	// Note: the system-level options have been processed and applied already!
  try {
	   if (args["bodies"]) {
		auto n = stoi(args("bodies")) - 2; // 2 have already been created
		add_bodies(n < 0 ? 0 : n); // Avoid possible overflow!
	}; if (args["interact"]) {
		interact_all();
	}; if (args["friction"]) {
		float f = stof(args("friction"));
		world().FRICTION = f;
	}; if (args["snd"]) {
		backend.audio.enabled(args("snd") != "off");
	}; if (args["zoom"]) {
		float factor = stof(args("zoom"));
		zoom(factor);
	}
	return true;
  } catch(...) {
	cerr << __FUNCTION__ ": ERROR processing/applying cmdline args!\n";
	return false;
  }

	// Init sounds (even if turned off; it may get turned back on by the user)
	clack_sound = backend.audio.add_sound(string(cfg.asset_dir + "sound/clack.wav").c_str());
	backend.audio.play_music(string(cfg.asset_dir + "music/default.ogg").c_str());
}

//----------------------------------------------------------------------------
bool OON::poll_and_process_controls()
{
	bool action = false;
	if (_ctrl_update_thrusters()) {
		action = true;
//		if (keystate(SPACE)) {
//			action = true;
//			exhaust_burst(player_entity_ndx(), 5);
//		}
	}
	// Allow this now irrespective of any engine firing:
	if (keystate(SPACE)) {
		action = true;
		exhaust_burst(player_entity_ndx(), 5);
	}

	if (_ctrl_update_pan())
		action = true;

	return action;
}

//----------------------------------------------------------------------------
void OON::up_thruster_start()    { entity(player_entity_ndx()).thrust_up.thrust_level(cfg.THRUST_FORCE); }
void OON::down_thruster_start()  { entity(player_entity_ndx()).thrust_down.thrust_level(cfg.THRUST_FORCE); }
void OON::left_thruster_start()  { entity(player_entity_ndx()).thrust_left.thrust_level(cfg.THRUST_FORCE); }
void OON::right_thruster_start() { entity(player_entity_ndx()).thrust_right.thrust_level(cfg.THRUST_FORCE); }
void OON::up_thruster_stop()     { entity(player_entity_ndx()).thrust_up.thrust_level(0); }
void OON::down_thruster_stop()   { entity(player_entity_ndx()).thrust_down.thrust_level(0); }
void OON::left_thruster_stop()   { entity(player_entity_ndx()).thrust_left.thrust_level(0); }
void OON::right_thruster_stop()  { entity(player_entity_ndx()).thrust_right.thrust_level(0); }

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
	view.offset = - entity(id).p * view.zoom;
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
		if (pan_step_x) pan_step_x -= sz::sign(pan_step_x);
		if (pan_step_y) pan_step_y -= sz::sign(pan_step_y);
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
	return world().add_body(std::forward<decltype(obj)>(obj));
}

//----------------------------------------------------------------------------
void OON::remove_body(size_t ndx)
{
	world().remove_body(ndx);
}

//----------------------------------------------------------------------------
size_t OON::add_body()
{
	const auto& cw = const_world();
	auto constexpr r_min = cw.CFG_GLOBE_RADIUS / 9;
	auto constexpr r_max = cw.CFG_GLOBE_RADIUS * 3;
	auto constexpr p_range = cw.CFG_GLOBE_RADIUS * 30;
	auto constexpr v_range = cw.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
//cerr << "Adding new object #" << cw.bodies.size() + 1 << "...\n";
	const auto& player = const_entity(player_entity_ndx());
	return add_body({
		.r = (float) (((float)rand() * (r_max - r_min)) / RAND_MAX ) //! suppress warning "conversion from double to float..."
				+ r_min,
		.p = { (rand() * p_range) / RAND_MAX - p_range/2 + player.p.x,
		       (rand() * p_range) / RAND_MAX - p_range/2 + player.p.y },
		.v = { (rand() * v_range) / RAND_MAX - v_range/2 + player.v.x * 0.05f,
		       (rand() * v_range) / RAND_MAX - v_range/2 + player.v.y * 0.05f },
		.color = 0xffffff & ((uint32_t) rand() * rand()),
	});
}

//----------------------------------------------------------------------------
void OON::remove_body()
{
	auto entities = entity_count();
	if (entities < 2) { // Leave the player "superglobe", so not just checking for empty()!
cerr <<	"No more \"free\" items to delete.\n";
		return;
	}

	auto ndx = 1/*leave the globe!*/ + rand() * ((entities-1) / (RAND_MAX + 1));
//cerr << "Deleting object #"	 << ndx << "...\n";
	assert(ndx < entities); // Note: entity indexes are 0-based
	assert(ndx > 0);        // Note: 0 is the player globe
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
	if (n == (unsigned)-1) n = entity_count();
	while (n--) remove_body();
}


//----------------------------------------------------------------------------
void OON::spawn(size_t parent_ndx, size_t n)
//!!??Should gradually become a method of the object itself
{
if (parent_ndx != player_entity_ndx()) cerr << "- INTERANL: Non-player object #"<<parent_ndx<<" is spawning...\n";

	// -> #41: Enable inheritance
	const auto& parent = const_entity(parent_ndx);
	for (size_t i = 0; i < n; ++i) {
		auto ndx = add_body();
		auto& newborn = entity(ndx);
		newborn.T = parent.T; // #155: Inherit T
	}
}

//----------------------------------------------------------------------------
void OON::exhaust_burst(size_t entity/* = 0*/, size_t n/* = 50*/)
{
/*!! Too boring with all these small particles:
	auto constexpr r_min = const_world().CFG_GLOBE_RADIUS / 10;
	auto constexpr r_max = const_world().CFG_GLOBE_RADIUS * 0.3;
	auto constexpr p_range = const_world().CFG_GLOBE_RADIUS * 2;
	auto constexpr v_range = const_world().CFG_GLOBE_RADIUS * 3; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
*/
	const auto& cw = const_world();
	auto constexpr r_min = cw.CFG_GLOBE_RADIUS / 10;
	auto constexpr r_max = cw.CFG_GLOBE_RADIUS * 0.5;
	auto constexpr p_range = cw.CFG_GLOBE_RADIUS * 5;
	auto constexpr v_range = cw.CFG_GLOBE_RADIUS * 10; //!!Stop depending on GLOBE_RADIUS so directly/cryptically!
	const auto& rocket = const_entity(entity);
	for (int i = 0; i++ < n;) {
		add_body({
			.r = (float) ((rand() * (r_max - r_min)) / RAND_MAX ) //! Suppress warning "conversion from double to float..."
					+ r_min,
 			//!!...Jesus, those literal pseudo dts here! :-o :)
			.p = { (rand() * p_range) / RAND_MAX - p_range/2 + rocket.p.x - rocket.v.x * 0.1f,
			       (rand() * p_range) / RAND_MAX - p_range/2 + rocket.p.y - rocket.v.y * 0.1f },
			.v = { (rand() * v_range) / RAND_MAX - v_range/2 + rocket.v.x * 0.1f,
			       (rand() * v_range) / RAND_MAX - v_range/2 + rocket.v.y * 0.1f },
			.color = (uint32_t) (float)0xffffff * rand(),
		});
	}
}
