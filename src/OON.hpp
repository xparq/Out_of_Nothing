#ifndef __OON__
#define __OON__

#include "Szim/SimApp.hpp"

#include <cassert>

//============================================================================
class OON : public Szim::SimApp
{
//----------------------------------------------------------------------------
// Config...
//----------------------------------------------------------------------------
protected:
	//!!Move the rest of these to the Model, too, for now:
	//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
	//!!(They will become props initialized from a real config!)
	static constexpr float CFG_THRUST_FORCE = 6e34f; // N (kg*m/s^2)
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!

//----------------------------------------------------------------------------
// API...
//----------------------------------------------------------------------------
public:

	auto&  player_model(unsigned player_id = 1) { assert(player_id == 1); return world.bodies[player_entity_ndx(player_id)]; }

	//------------------------------------------------------------------------
	// Player actions...

	// - Gameplay:
	virtual void spawn(size_t n = 1, size_t parent_ndx = 0); //!! required: 0 == globe_ndx
	virtual void exhaust_burst(size_t n = 50);

	auto interact_all(bool state = true)  { world._interact_all = state; }
	auto toggle_interact_all()  { interact_all(!world._interact_all); }

	//!!These should be idempotent, to tolerate keyboard repeats (which could be disabled but may be problematic)!
	//!!Also -> #105: sanitize thrusters...
	void    up_thruster_start();
	void  down_thruster_start();
	void  left_thruster_start();
	void right_thruster_start();
	void    up_thruster_stop();
	void  down_thruster_stop();
	void  left_thruster_stop();
	void right_thruster_stop();

	// - View control:
	void pan(Math::Vector2f delta);
	void pan_x(float delta);
	void pan_y(float delta);
	void pan_reset();
	void pan_to_entity(size_t id);
	void pan_to_player(unsigned player_id = 1) { assert(player_id == 1); return pan_to_entity(player_entity_ndx(player_id)); }
	auto zoom_in  () { constexpr static auto factor =    1 + CFG_ZOOM_CHANGE_RATIO;  zoom(factor); }
	auto zoom_out () { constexpr static auto factor = 1/(1 + CFG_ZOOM_CHANGE_RATIO); zoom(factor); }
	void zoom(float factor);

	//------------------------------------------------------------------------
	// Internals: not even user actions (Well, some still are, basically for testing.)
	//!!Make a proper distinction between these and the player/user actions!
	//!!(One thing's that those tend/should go through the UI, whereas these shouldn't.)
	virtual size_t add_body(Model::World::Body&& obj);
//!!	virtual size_t add_body(Model::World::Body const& obj);
	virtual void   remove_body(size_t ndx);
	size_t add_body(); // add a random one
	void   remove_body(); // delete a random one
	void   add_bodies(size_t n);
	void   remove_bodies(size_t n = -1); // -1 -> all

	size_t player_entity_ndx([[maybe_unused]] unsigned player_id = 1) const { assert(player_id == 1); return globe_ndx; }

	virtual bool poll_and_process_controls() override; // true if there was any input

	bool _ctrl_update_thrusters(); // true if any engine is firing
	bool _ctrl_update_pan(); // true if panning was requested

//----------------------------------------------------------------------------
// Virtuals...
//----------------------------------------------------------------------------
private:
	//------------------------------------------------------------------------
	// - Implemented:

	// Model event callback implementations... //!!Then move it to some more "modelly place" later, as things will get more complicated.
	virtual void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...) override;

	//------------------------------------------------------------------------
	// - Introduced:
	virtual void post_zoom_hook([[maybe_unused]] float factor) {}

//----------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	using SimApp::SimApp;
	OON(const OON&) = delete;

//----------------------------------------------------------------------------
// Data / Internals...
//----------------------------------------------------------------------------
protected:
	constexpr static auto CFG_PAN_STEP = 5; // "SFML pixel"
	constexpr static auto CFG_ZOOM_CHANGE_RATIO = 0.25f; // 25%

	// These will be reset to +/-CFG_PAN_STEP whenever starting to pan:
	float pan_step_x = 0, pan_step_y = 0;

	size_t globe_ndx = 0; // paranoid safety init (see _setup()!)
	size_t clack_sound = 0; // paranoid safety init (see _setup()!)
};

#endif // __OON__
