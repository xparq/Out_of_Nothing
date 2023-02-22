#include "SimApp.hpp"

#include <cassert>
#include <string>
	using std::string, std::to_string;
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#include <format>
	using std::format;
#include <iostream>
	using std::cerr, std::endl;

//============================================================================
//----------------------------------------------------------------------------
//!!These sould be atomic/blocking, but... meh... ;)
//!!These are being called currently from a locked section of the event loop anyway,
//!!but it's a crime relying on it here! Either document that this is not thread-safe,
//!!or make it safe here (how exactly, not introducing (and then imposing) specifics?)!
Model::World const& SimApp::get_world() const { return world; }
Model::World& SimApp::get_world() { return world; }
void SimApp::set_world(Model::World const& w) { world = w; }
void SimApp::set_world(Model::World & w) { world = w; }

//----------------------------------------------------------------------------
bool SimApp::save_snapshot(unsigned slot_id) // starting from 1, not 0!
{
	//!!A kinda alluring abstraction would be SimApp not really having its own state
	//!!(worth saving, beside the model world), leaving all that to descendants...
	//!!But I suspect it's unfounded; at least I can't see the higher principle it
	//!!could be derived from... What I do see, OTOH, is the hassle in the App class
	//!!chain to actually deal with saving/loading all the meta/supplementary state...
/*
	using namespace MEMDB;
	assert(slot_id > 0 && slot_id <= MAX_WORLD_SNAPSHOTS); //!!should become a runtime "filename OK" check

	auto slot = slot_id - 1; //! internally they are 0-based tho...
	decltype(saved_slots) slot_bit = 1 << slot;
	if (saved_slots & slot_bit) {
		cerr << "- WARNING: Overwriting previously saved state at slot #" << slot_id << "!...\n";
	}

	world_snapshots[slot] = get_world(); // :)
	saved_slots |= slot_bit;
*/
	Model::World snapshot = get_world();

	string fname = snapshot_filename(slot_id);
	string OVERALL_FAIL = "ERROR: Couldn't save snapshot to file \""; OVERALL_FAIL += fname + "\"\n";
	ofstream out(fname, ios::binary);
	if (!out || out.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

//!!Redesign this proc. so that this can be done from a descendant's save_...() override:
//!!out << "BUILD_ID = " << ::BUILD_ID << endl;

	if (!snapshot.save(out)) {
		cerr << OVERALL_FAIL;
		return false;
	}

	cerr << "World state saved to slot #" << slot_id << ".\n";
	return true;
}

//----------------------------------------------------------------------------
bool SimApp::load_snapshot(unsigned slot_id) // starting from 1, not 0!
{
/*
	using namespace MEMDB;
	assert(slot_id > 0 && slot_id <= MAX_WORLD_SNAPSHOTS); //!!should become a runtime "filename OK" check

	auto slot = slot_id - 1; //! internally they are 0-based tho...
	decltype(saved_slots) slot_bit = 1 << slot;
	if (! (saved_slots & slot_bit)) {
		cerr << "- WARNING: No saved state at slot #" << slot_id << " yet!\n";
		return false;
	}
	set_world(world_snapshots[slot]);
	cerr << "World state loaded from slot " << slot_id << ".\n";
*/
	Model::World snapshot;

	string fname = snapshot_filename(slot_id);
	string OVERALL_FAIL = "ERROR: Couldn't load snapshot from file \""; OVERALL_FAIL += fname + "\"\n";
	ifstream in(fname, ios::binary);
	if (!in || in.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

//!!Redesign this proc. so that this can be done from a descendant's load_...() override:
//!!in >> BUILD_ID...

	if (!Model::World::load(in, &snapshot)) {
		cerr << OVERALL_FAIL;
		return false;
	}

	set_world(snapshot);

	cerr << "World state loaded from slot #" << slot_id << ".\n";
	return true;
}

//----------------------------------------------------------------------------
bool SimApp::collide_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2, float distance)
{w, obj1, obj2, distance;
	//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
	//!!...body->p -= ds...;
	return false;
}

bool SimApp::touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2)
{w, obj1, obj2;
	return false;
}

// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
//!!The model should also pass the physical property/condition ("event type") that made it think these may interact!
//!!A self-documenting alternative would be calling a matching function for each known such event,
//!!but that might be prohibitively expensive in that tight loop, especiall if most callbacks would
//!!just do nothing.
//!!NOTE: This will anyway change to the objects themselves being notified (not the game "superclass")!
void SimApp::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
	//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
}
