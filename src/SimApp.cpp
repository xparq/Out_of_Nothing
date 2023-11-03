#define SAVE_COMPRESSED

#include "SimApp.hpp"

#include "extern/toml++/sz-toml.hpp" // Proxy header with custom cfg.

#include <string>
	using std::string, std::to_string;
	using namespace std::string_literals;
#include <string_view>
	using std::string_view;
#include "sz/fs.hh"
	using sz::dirname;
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#ifdef SAVE_COMPRESSED
#   include "extern/zstd/zstd.h"
#   include <sstream>
    using std::ostringstream, std::stringstream;
#   include <string_view>
    using std::string_view;
#   include <memory>
    using std::make_unique_for_overwrite;
#   include <cstddef>
    using std::byte; //!! It's fucked up in C++ tho: a byte[] buffer can't be used for file IO... Excellent.
#endif
#include <format>
	using std::format;
#include <iostream>
	using std::cerr, std::endl;
#include <stdexcept>
	using std::runtime_error;

//============================================================================
//----------------------------------------------------------------------------
SimApp::SimApp(const char* cfgfile)
{
	if (!cfgfile || !*cfgfile)
		cfgfile = DEFAULT_CFG_FILE;

	// Relative paths will be rooted to the dir of 'cfgfile' by default,
	// i.e. unless it's specifically set in the config
	//!!Move to unilang:
	cfg.cfg_dir = dirname(cfgfile);
	//!!auto basename = fs::path(cfgfile).filename().string();

	cfg.asset_dir = sz::getcwd() + "/asset/"; //!! Trailing / still required...

	if (auto config = toml::parse_file(cfgfile); !config) {
		throw runtime_error("Failed to load config: "s + cfgfile);
	} else {
		cfg.asset_dir = config["fs-layout"]["asset_dir"].value_or(cfg.asset_dir);
		//int x = config["?"]["number"].value_or(0);
	}

cerr <<	"DBG> current dir: " << sz::getcwd() << '\n';
cerr <<	"DBG> asset_dir: " << cfg.asset_dir << '\n';

}


//----------------------------------------------------------------------------
void SimApp::pause(bool newstate)
{
	_time_paused = newstate;
	on_pause_changed(newstate);
}


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

	//!! Note: perror("") may just print "No error" even if the stream in failure mode! :-/

#ifdef SAVE_COMPRESSED
	ofstream file(fname, ios::binary);
	if (!file || file.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

	ostringstream out(ios::binary); //!!??Why did it fail with plain stringstream?!?!?!
	//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
	//!!out << "BUILD_ID = " << ::BUILD_ID << endl;
	if (!snapshot.save(out)) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

	//file << out.view();

	// Compress (the whole blob in one go... <- !!IMPROVE)
	auto data_size = out.tellp(); // or out.view().size()
	auto cbuf_size = ZSTD_compressBound(data_size);
	auto cbuf = make_unique_for_overwrite<char[]>(cbuf_size);
	auto cfile_size = ZSTD_compress(cbuf.get(), cbuf_size, out.view().data(), data_size, 9);

	if (!file.write(cbuf.get(), cfile_size) || file.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

#else // !SAVE_COMPRESSED
	ofstream out(fname, ios::binary);
	if (!out || out.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}
	//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
	//!!out << "BUILD_ID = " << ::BUILD_ID << endl;
	if (!snapshot.save(out)) {
		cerr << OVERALL_FAIL;
		return false;
	}
#endif // SAVE_COMPRESSED

	assert(out && !out.bad());

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

#ifdef SAVE_COMPRESSED
	ifstream file(fname, ios::binary);
	if (!file || file.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

	stringstream in(ios::in|ios::out|ios::binary);
	in << file.rdbuf();
	if (!in || in.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

	// Decompress (the whole blob in one go... <- !!IMPROVE)
	auto cbuf_size = in.view().size();
	auto cbuf = in.view().data();
	auto data_size = ZSTD_getFrameContentSize(cbuf, cbuf_size);
	auto data = make_unique_for_overwrite<char[]>(data_size);
	auto dsize = ZSTD_decompress(data.get(), data_size, cbuf, cbuf_size);
	assert(dsize == data_size);

	//!!Only in c++26: in.str(string_view((char*)data.get(), data_size)); // or: reset, then: in.write(data.get(), data_size);
	in.seekp(0, in.beg); // out
	in.write(data.get(), data_size);
	in.seekg(0, in.beg); // in

	if (!in || in.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

	if (!Model::World::load(in, &snapshot)) {
		cerr << OVERALL_FAIL;
		return false;
	}

	if (!in || in.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

#else // !SAVE_COMPRESSED
	ifstream in(fname, ios::binary);
	if (!in || in.bad()) {
		cerr << OVERALL_FAIL;
		perror("");
		return false;
	}

	//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
	//!!in >> BUILD_ID...

	if (!Model::World::load(in, &snapshot)) {
		cerr << OVERALL_FAIL;
		return false;
	}
#endif // SAVE_COMPRESSED

	assert(in && !in.bad());

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
