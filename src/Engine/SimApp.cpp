#define SAVE_COMPRESSED

#include "SimApp.hpp"

#include "View/ScreenView.hpp"

#include "sz/stringtools.hh"
//	using sz::to_bool

#include <string>
	using std::string, std::to_string;
//	using std::stoul, std::stof;
//	using namespace std::string_literals;
//#include <string_view>
//	using std::string_view;
#include "sz/fs.hh"
	using sz::prefix_if_rel;
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#ifdef SAVE_COMPRESSED
#   include "extern/zstd/zstd.h"
#   include <sstream>
    using std::ostringstream, std::stringstream;
#   include <memory>
    using std::make_unique_for_overwrite;
#   include <cstddef>
    using std::byte; //!! It's fucked up in C++ tho: a byte[] buffer can't be used for file IO... Excellent.
#endif
#include <format>
	using std::format;
#include <iostream>
	using std::cerr, std::cout, std::endl;
//#include <stdexcept>
//	using std::runtime_error;


using namespace Szim;
//============================================================================
void SimApp::show_cmdline_help(const Args& args, const char* banner)
{
	std::string descr;

	if (!banner) {
		banner = "\"Intuition\" - Simulation engine and app. dev. framework\n";
	}

	descr = R"(
Usage:)"; descr += args.exename(); descr += " [options...]\n";
	descr += R"(
Options:

  -h | -? | --help
  	Show this help.
  -V
  	Show version information.
)";

	cout << banner << descr;
}

//----------------------------------------------------------------------------
void SimApp::init()
//
// Internal init, called from the ctor...
//
// >>>  NO VIRTUAL DISPATCH IS AVAILABLE HERE YET!  <<<
//
{
	// Guard against multiple calls (which can happen if not overridden):
	static auto done = false; if (done) return; else done = true; // The exit code may have already been set!

	// Apply the config...

	// Misc. fixup that should've been in the ctors, but C++...

	//
	// Some args aren't/can't/shoudn't be handled by SimAppConfig itself...
	//
	if (args["snd"])
		backend.audio.enabled(sz::to_bool(args("snd"), sz::str::empty_is_true));

	// Time control...
	iterations.max(cfg.iteration_limit);
	if (cfg.fixed_model_dt_enabled)
		time.last_model_Δt = cfg.fixed_model_dt; // Otherwise no one might ever init this...

	// Sessions...
	if (!sz::to_bool(args("session-autosave"), sz::str::empty_is_true) // explicitly set to false?
	    || args["session-no-autosave"])
		session.set_autosave(false);

	if (!args("session-save-as").empty()) // Even if autosave disabled. (Could be reenabled later, or manual save...)
		session.set_save_as_filename(args("session-save-as"));

cerr << "<<< SimApp Engine/API initialized. >>>\n\n";
}

//----------------------------------------------------------------------------
void SimApp::done()
//
// Internal cleanup, called from the dtor...
//
// >>>  NO VIRTUAL DISPATCH IS AVAILABLE HERE ANY MORE!  <<<
//
{
	// Guard against multiple calls (which can happen if not overridden):
	static auto done = false; if (done) return; else done = true; // The exit code may have already been set!

cerr << "\n<<< SimApp Engine/API shutting down... >>>\n";
}

//----------------------------------------------------------------------------
int SimApp::run()
{
/*
	//!!
	//!! I'VE JUST DISABLED THIS LEFTOVER CRUFT (SHOULD BE IN THE BACKEND ANYWAY),
	//!! SO THAT I COULD MOVE run() TO THE NON-BACKEND-SPECIFIC PART OF SimApp FINALLY.
	//!! AND THIS CHANGE HAS APPARENTLY BEEN REDUNDANT...
	//!! I'M STILL LEAVING IT HERE, AS THE UNDERLYING ISSUE (#190) REMAINS UNRESOLVED.
	//!
	//! The event loop will block and sleep.
	//! The update thread is safe to start before the event loop, but we'd also need to
	//! draw things before the first event, so we have to release the SFML (OpenGL) Window (crucial!),
	//! and unfreeze the update thread (which would wait on the first event by default).
	//!
	//!! [THINGS HAVE CHANGED SINCE THEN! E.g. events are polled now also in threaded mode,
	//!! so no blocking before the first one. And that "crucial" GL release seems to not
	//!! make any difference whatsoever now... Everything just works as before without it.
	//!! -- BUT THAT MAY BE A LUCKY COINCIDENCE OF THREAD SCHEDULING! :-o
	//!! Also: what does/did the 1st-event "freeze" have to do with setActive()?!]
	//!!
	if (!((SFML_Backend&)backend).SFML_window().setActive(false)) { //https://stackoverflow.com/a/23921645/1479945
		cerr << "\n- [main] sf::setActive(false) failed, WTF?! Terminating.\n";
		return -1;
	}
*/
	if (terminated()) // Terminated during construction? (e.g. by the default init())...
		return exit_code();

	init(); // Unlike the ctor, this calls the override (or the "onced" NOOP default, if none)

	if (terminated()) // init() may have been "aborted"...
		return exit_code();
		// NOTE: If there's no init() override (-- weird, but who knows! :-o ),
		// but there *is* an overridden done() (-- wow, even weirder!!! :) ),
		// that will be called normally, as if the default init was the client's.

	cerr << "> Engine: Client app initialized. Starting main loop...\n";

	ui_event_state = SimApp::UIEventState::IDLE;

#ifndef DISABLE_THREADS
	std::thread game_state_updates(&SimApp::update_thread_main_loop, this);
		//! NOTES:
		//! - When it wasn't a member fn, the value vs ref. form was ambiguous and failed to compile!
		//! - The thread ctor would *copy* its params (by default), which would be kinda wonky for the entire app. ;)
#endif

	event_loop();

#ifndef DISABLE_THREADS
//cerr << "TRACE - before threads join\n";
	game_state_updates.join();
#endif

	cerr << "> Engine: Main loop finished. Cleaning up client app...\n";

	done(); // Unlike the dtor, this calls the override (or the "onced" NOOP default if none)

	return exit_code();
}


//----------------------------------------------------------------------------
void SimApp::request_exit(int exit_code)
{
	_terminated = true;
	_exit_code = exit_code;
}

//----------------------------------------------------------------------------
void SimApp::pause(bool newstate)
{
	time.paused = newstate;
	pause_hook(newstate);
}

bool SimApp::toggle_pause() { pause(!paused()); return paused(); }

bool SimApp::toggle_fixed_model_dt()
{
	//!!! THREADING !!!

	cfg.fixed_model_dt_enabled = !cfg.fixed_model_dt_enabled;

	//!! + this, but only to support the debug HUD! :) :-o
	if (cfg.fixed_model_dt_enabled) time.last_model_Δt = cfg.fixed_model_dt;

	return cfg.fixed_model_dt_enabled;
}


//----------------------------------------------------------------------------
//!! Updating is not yet (inherently) thread-safe!
//!! These sould be atomic/blocking/mutex-protected/...!
//!! (Delegating that to all the various World::mutators, and imposing deps.
//!! on threading (etc.) mechanics there feels like a terrible idea!)
//!! While update() and load() are called currently from a locked section of the
//!! event loop anyway, it's a crime to rely on just that!
      Model::World& SimApp::world()       { return _world; }
const Model::World& SimApp::world() const { return _world; }
const Model::World& SimApp::const_world() { return _world; }
void SimApp::set_world(Model::World const& w) { _world = w; }


//----------------------------------------------------------------------------
bool SimApp::quick_save_snapshot(unsigned slot_id) // starting from 1, not 0!
{
/*
	using namespace MEMDB;
	assert(slot_id > 0 && slot_id <= MAX_WORLD_SNAPSHOTS); //!!should become a runtime "filename OK" check

	auto slot = slot_id - 1; //! internally they are 0-based tho...
	decltype(saved_slots) slot_bit = 1 << slot;
	if (saved_slots & slot_bit) {
		cerr << "- WARNING: Overwriting previously saved state at slot #" << slot_id << "!...\n";
	}

	world_snapshots[slot] = world(); // :)
	saved_slots |= slot_bit;
*/
	return save_snapshot(
		snapshot_filename(slot_id, cfg.quick_snapshot_filename_pattern.c_str()).c_str());
}

//----------------------------------------------------------------------------
bool SimApp::save_snapshot(const char* unsanitized_filename)
{
	//!!A kinda alluring abstraction would be SimApp not really having its own state
	//!!(worth saving, beside the model world), leaving all that to descendants...
	//!!But I suspect it's unfounded; at least I can't see the higher principle it
	//!!could be derived from... What I do see, OTOH, is the hassle in the App class
	//!!chain to actually deal with saving/loading all the meta/supplementary state...

	string fname = sz::prefix_if_rel(cfg.session_dir, unsanitized_filename);

	string OVERALL_FAIL = "ERROR: Couldn't save snapshot to file \""; OVERALL_FAIL += fname + "\"\n";

	Model::World snapshot = world();
	//!! OK, now we could start a low-priority background thread to actually save the snapshot...

	//!! Note: perror("") may just print "No error" even if the stream is in failure mode! :-/

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

	cerr << "World state saved to \"" << fname << "\".\n";
	return true;
}


//----------------------------------------------------------------------------
bool SimApp::quick_load_snapshot(unsigned slot_id) // starting from 1, not 0!
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
	return load_snapshot(
		snapshot_filename(slot_id, cfg.quick_snapshot_filename_pattern.c_str()).c_str());
}

//----------------------------------------------------------------------------
bool SimApp::load_snapshot(const char* unsanitized_filename)
{
	string fname = sz::prefix_if_rel(cfg.session_dir, unsanitized_filename);

	string OVERALL_FAIL = "ERROR: Couldn't load snapshot from file \""; OVERALL_FAIL += fname + "\"\n";

	//!! We could start a low-priority background thread
	//!! to load a world state into a buffer first, and then
	//!! copy it over the live instance when ready...

	Model::World snapshot; // The input buffer

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

	cerr << "World state loaded from \"" << fname << "\".\n";
	return true;
}


//----------------------------------------------------------------------------
bool SimApp::is_entity_at_viewpos(size_t entity_id, float x, float y) const // virtual
{
	const auto& e = entity(entity_id);
	//!! Check if view pos is cached first! (But that lookup could be even more expensive... MEASURE!)
	//!! Actully, in OONApp_sfml it is -- make this "tunnelable"!...
	const auto& camera = main_view().camera();
	auto ep = camera.world_to_view_coord(e.p);
	//!! ... = e.bounding_box();
	auto box_R = e.r * camera.scale(); //!! Not a terribly robust method to get that size...
	auto distance = Math::mag2(ep.x - x, ep.y - y); //!! Sigh... #327
//cerr << "---> ...checking click at ("<<x<<", "<<y<<") against entity #"<<i<<" at ("<<ep.x<<", "<<ep.y<<")...\n";

	if (distance <= box_R) {
//cerr << "- FOUND entity #" << i << "!\n";
		return true;
	} else  return false;
}

//----------------------------------------------------------------------------
bool SimApp::entity_at_viewpos(float x, float y, size_t* entity_id OUT) const // virtual
{
	for (size_t i = entity_count(); i-- != 0;) { //!! Poor man's Z-order... Override for less hamfisted ways!
		if (is_entity_at_viewpos(i, x, y)) {
			*entity_id = i;
			return true;
		}
	}
	return false;
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


//----------------------------------------------------------------------------
void SimApp::toggle_fullscreen()
{
	is_fullscreen = !is_fullscreen;
	backend.hci.switch_fullscreen(is_fullscreen);
	auto width  = backend.hci.window().width;
	auto height = backend.hci.window().height;

	/// Refresh engine state...
	main_view().resize(width, height);

	// OK, notify the client:
	onResize(backend.hci.window().width, backend.hci.window().height);
}

//----------------------------------------------------------------------------
unsigned SimApp::fps_throttling(unsigned new_fps_limit/* = -1u*/)
{
	if (new_fps_limit != (unsigned)-1) {
	// Set...
cerr << "DBG> "<<__FUNCTION__<<": Setting FPS limit to "<<new_fps_limit<<"\n";
		backend.hci.set_frame_rate_limit(new_fps_limit); // 0: no limit
	}

	// Query...
	return backend.hci.get_frame_rate_limit(); // C++ converts it to false when 0 (no limit)
}

void SimApp::fps_throttling(bool onoff)
{
	fps_throttling(unsigned(onoff ? cfg.fps_limit : 0));
}
