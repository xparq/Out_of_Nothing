#ifndef _SADFM987HUYY09786UHYTIUHGJM9587CUV_
#define _SADFM987HUYY09786UHYTIUHGJM9587CUV_

#include "SimApp.hpp"
#include "SessionManager.hpp"

#include <utility> // exchange
//#include <concepts> // derived_from
//#include <type_traits>
#include <cassert>


#include "Szim/Math.hpp" //!! For is_entity_at_viewpos() (calling mag2)... Move it out from here!


//!!
//!! Pasted here for save/load:
//!! CLEAN UP! Move it out from here!
//!!
#include "diag/Log.hpp"
#include "diag/Error.hpp"
#include "sz/diag/DBG.hh"


#include <string>
	using std::string, std::to_string;
	using namespace std::string_literals;
//#include <string_view>
//	using std::string_view;
#include "sz/sys/fs.hh"
	using sz::fs::prefix_by_intent;
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#include <cerrno>
#include <cstring> // strerror(errno)

#ifndef DISABLE_SNAPSHOT_COMPRESSION
#   include "extern/zstd/zstd.h"
#   include <sstream>
    using std::ostringstream, std::stringstream;
#   include <memory>
    using std::make_unique_for_overwrite;
#   include <cstddef>
    using std::byte; //!! It's fucked up in C++ tho: a byte[] buffer can't be used for file IO... Excellent.
#endif // DISABLE_SNAPSHOT_COMPRESSION












namespace Szim {


//----------------------------------------------------------------------------
template <typename C>
concept AppTraitsC = requires(C c)
{
	typename C::WorldT;
	requires std::derived_from<typename C::WorldT, Szim::Model::Core::World>;
};


//----------------------------------------------------------------------------
template <AppTraitsC AppCfg>
class App : public SimApp
{
protected:

	SessionWrapper<App<AppCfg>> session_manager; //! Typed wrapper of SimApp.session

public:
	// Lift the base ctors:
	//!! BUT... NO! Those WON'T BE ABLE to also initialize any app-typed data managed here!
	//!!using SimApp::SimApp;
	App(const RuntimeContext& rt, int argc, char** argv, View::ScreenView& main_view)
		: SimApp(rt, argc, argv, main_view)
		, session_manager(SimApp::session)
	{
		//!!?? Defer to init()!
		//! Deleted in ~SimApp() (or maybe "also" in SimApp::done()?)
		set_world(create_world());
	}

	// "Internalize" the app types...
	using WorldT  = AppCfg::WorldT;
	using EntityT = AppCfg::EntityT;
	//!!using PlayerT = typename AppCfg::Player;
	//!!...

protected:
	WorldT* create_world() override
	{
		return new WorldT(*this);
	}

public:
	// ----------------------------------------------------------------------
	// Takes ownership, nulls wptr.
	// Usage: app.set_world(&wptr);
	// Note: WorldT** and Szim::World** are UNRELATED pointer types, and can't
	// upcast at all, hence the need for this exact-matching wrapper.
	// ----------------------------------------------------------------------
	void set_world(WorldT** wptr)
	{
		SimApp::set_world(std::exchange(*wptr, nullptr));
	}

	// ----------------------------------------------------------------------
	// "Sink" for rvalues, rejecting lvalues. Takes ownership. 
	// Usage: app.set_world(app.create_world());
	//        app.set_world(std::move(wptr)); // Explicit opt-in for lvals
	// ----------------------------------------------------------------------
	void set_world(WorldT*&& wptr)
	{
		// Since there's no extra ptr indirection here (this is practically
		// by-value semantics), C++ will implicitly convert Derived* to Base*.
		SimApp::set_world(wptr);
	}	

	void set_world(WorldT*& wptr) = delete; // Prevent "catastrophic" bind to temporary implicitly upcasted from WorldT lval...

#ifdef TEST_COMPILATION
	void DUMMY_COMPILE_TEST()
	{
		set_world(new WorldT);
		set_world(create_world());
		auto w = create_world();
		set_world(&w);
		set_world(std::move(w));
		set_world(w); // SHOULD FAIL!
	}
#endif

	      WorldT& world()       { return static_cast<      WorldT&>(__world()); }
	const WorldT& world() const { return static_cast<const WorldT&>(__world()); }
	// Explicit const World& of non-const SimApp (to spare a cast)
	const WorldT& const_world() { return static_cast<const WorldT&>(__const_world()); }

	//!! ADD DEBUG-MODE BOUNDS-CHECKING FOR THESE!
	// Thread-safe, slower access:
	      EntityT& entity(EntityID index)       { return *world().bodies[index]; }
	const EntityT& entity(EntityID index) const { return *world().bodies[index]; }
	const EntityT& const_entity(EntityID index) { return *const_world().bodies[index]; }
	/*!! This feels misguided, but keeping it as a reminder...
	// Unprotected, faster access (when already "locked"):
	      EntityT& _entity(EntityID index)       { return *world_->bodies[index]; }
	const EntityT& _entity(EntityID index) const { return *world_->bodies[index]; }
	const EntityT& _const_entity(EntityID index) { return *world_->bodies[index]; }
	!!*/

	      EntityT& player_entity(PlayerID p = 1)       { assert(entity_count() > player_entity_ndx(p)); return entity(player_entity_ndx(p)); }
	const EntityT& player_entity(PlayerID p = 1) const { assert(entity_count() > player_entity_ndx(p)); return entity(player_entity_ndx(p)); }


	//----------------------------------------------------------------------------
	//! These are only `virtual` to formalize the intent for optional overriding!
	//! But the overridden ones are accessed via a templating, not the vtable (i.e.
	//! devirtualized)!
	//! So, the "overrides" must be made public, just like these!
	//! (If they wouldn't, they'd even trigger "shadows" warnings when overloaded.)
	virtual EntityID add_entity(EntityT&& temp)
	{
		return world().add_body(std::forward<decltype(temp)>(temp)); //!!?? That forward is redundant here?
	}

	virtual EntityID add_entity(const EntityT& src)
	{
		return world().add_body(src);
	}

	virtual void remove_entity(EntityID id)
	{
		world().remove_body(id);
	}

//!!	PlayerID add_player(Player&& tempp); // Calls a virtual hook to let the app finish it...
	virtual PlayerID add_player(
		EntityT&& model,
		Avatar& avatar,
		VirtualController& controls
	) = 0; //!! Ugh... Refine!
	virtual void   remove_player(PlayerID player_id) = 0; //! Should then be virtual, too (just like destructors)!


	//----------------------------------------------------------------------------
	bool is_entity_at_viewpos(EntityID id, float x, float y) const override
	// Can't be in the World abstraction, as that doesn't know anything about displaying things...
	// Should be a feature of the app-level customization layer of the UI, actually (probably)!
	{
		const auto& e = entity(id);
		//!! Check if view pos is cached first! (But that lookup could be even more expensive... MEASURE!)
		//!! Actually, in OONApp_sfml it is -- make this "tunnelable"!...
		const auto& camera = main_view().camera();
		auto ep = camera.world_to_view_coord(e.p);
		//!! ... = e.bounding_box();
		auto box_R = e.r * camera.scale(); //!! Not a terribly robust method to get that size...
		auto distance = ::Math::mag2(ep.x - x, ep.y - y); //!! #327: extract Math from the Model
	//DBG "---> ...checking click at ("<<x<<", "<<y<<") against entity #"<<i<<" at ("<<ep.x<<", "<<ep.y<<")...";

		if (distance <= box_R) {
	//DBG "- FOUND entity #" << i;
			return true;
		} else  return false;
	}





	//----------------------------------------------------------------------------
	// APP (actually MODEL, for now!) SAVE/LOAD...
	//
	// 1 <= slot <= MAX_WORLD_SNAPSHOTS
	// See cfg.quick_snapshot_filename_pattern!
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	bool quick_save_snapshot(unsigned slot_id = 1) // starting from 1, not 0!
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
			SimApp::snapshot_filename(slot_id, cfg.quick_snapshot_filename_pattern.c_str()).c_str());
	}

	//----------------------------------------------------------------------------
	bool quick_load_snapshot(unsigned slot_id = 1) // starting from 1, not 0!
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
			SimApp::snapshot_filename(slot_id, cfg.quick_snapshot_filename_pattern.c_str()).c_str());
	}

	//----------------------------------------------------------------------------
	bool save_snapshot(const char* unsanitized_filename, SaveOpt flags = UseDefaults)
	{
		//!!A kinda alluring abstraction would be SimApp not really having its own state
		//!!(worth saving, beside the model world), leaving all that to descendants...
		//!!But I suspect it's unfounded; at least I can't see the higher principle it
		//!!could be derived from... What I do see, OTOH, is the hassle in the App class
		//!!chain to actually deal with saving/loading all the meta/supplementary state...

		string fname = sz::fs::prefix_by_intent(cfg.session_dir, unsanitized_filename);

		auto print_error = [&fname](string alt_msg = "<unset>") {
			string msg;
			if (alt_msg != "<unset>") msg = alt_msg;
			else msg = "Couldn't save snapshot to file \"" + fname + "\"";
			if (errno) { if (msg != "") msg += "\n  - ";
				msg += "CRT error: \""s + std::strerror(errno); /*errno = 0;*/ }
			if (msg != "") Error(msg);
		};

		const auto& snapshot = const_world(); //!! const_world()
		//!!
		//!! We could now start a low-priority background thread to actually do the saving...
		//!! But it's instantaneous for now, so no point. And even if there was a spike,
		//!! if anyone ever complained about that scenario, the technically sound solution
		//!! would be just banning them. ;)
		//!!

		//!! Note: perror("") may just print "No error" (for errno == 0) even if the stream is in failure mode! :-/

	#ifndef DISABLE_SNAPSHOT_COMPRESSION
		if (flags == UseDefaults ? cfg.save_compressed : flags & SaveOpt::Compress) { // Compressed
			ofstream file(fname, ios::binary);
			if (!file || file.bad()) { print_error(); return false; }

			ostringstream out(ios::binary); //!!??Why did it fail with plain stringstream?!?!?!
			//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
			//!!out << "BUILD_ID = " << ::BUILD_ID << endl;
			if (!snapshot.save(out)) {
				print_error();
				return false;
			}

			//file << out.view();

			// Compress (the whole blob in one go... <- !!IMPROVE)
			auto data_size = out.tellp(); // or out.view().size()
			auto cbuf_size = ZSTD_compressBound(data_size);
			auto cbuf = make_unique_for_overwrite<char[]>(cbuf_size);
			auto cfile_size = ZSTD_compress(cbuf.get(), cbuf_size, out.view().data(), data_size, 9);

			if (!file.write(cbuf.get(), cfile_size) || file.bad()) {
				print_error();
				return false;
			}
			assert(out && !out.bad());
		} else { // Not compressed
	#endif
			ofstream out(fname, ios::binary);
			if (!out || out.bad()) {  print_error(); return false; }

			//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
			//!!out << "BUILD_ID = " << ::BUILD_ID << endl;
			if (!snapshot.save(out)) {
				print_error();
				return false;
			}
			assert(out && !out.bad());
	#ifndef DISABLE_SNAPSHOT_COMPRESSION
		} // Compressed?
	#endif

		Note("World state saved to \"" + fname + "\".");
		return true;
	} // save


	//----------------------------------------------------------------------------
	//!! This is probably not a working setup; inversion of control (hook(s)/callback(s)),
	//!! or orchestrating typed/untyped work pieces here would be required!
	bool load_snapshot(const char* unsanitized_filename)
	{
		// The input "buffer":
		auto new_world = create_world();

		if (!_load_snapshot_into_tmp(unsanitized_filename, new_world)) {
			Error("Load failed.");
			delete new_world;
			return false;
		}

		set_world(&new_world);
		LOGN << "Loaded world activated.";

		//DBG "Calling the app's on_snapshot_loaded()...";
		on_snapshot_loaded();
		//DBG_(entity_count());
		//DBG_(main_view().shape_count());

		return true;
	}

	//!!OLD: bool SimApp::load_snapshot(const char* unsanitized_filename)
	bool _load_snapshot_into_tmp(const char* unsanitized_filename, WorldT* world_in)
	{
		string fname = sz::fs::prefix_by_intent(cfg.session_dir, unsanitized_filename);

	//	auto print_error = [&fname](string alt_msg = "<unset>") {
	//		if (alt_msg != "<unset>") cerr << alt_msg << (alt_msg.empty() ? "":"\n"); // Allow "" for no custom msg!
	//		else cerr << "- ERROR: Couldn't load snapshot from file \"" << fname << "\"" << '\n';
	//		if (errno) { cerr << "  (CRT error: \""<< std::strerror(errno) << "\")\n"; /*errno = 0;*/ }
	//	};
		auto print_error = [&fname](string alt_msg = "<unset>") {
			string msg;
			if (alt_msg != "<unset>") msg = alt_msg;
			else msg = "Couldn't load snapshot from file \"" + fname + "\"";
			if (errno) { if (msg != "") msg += "\n  - ";
				msg += "CRT error: \""s + std::strerror(errno); /*errno = 0;*/ }
			if (msg != "") Error(msg);
		};

		//!! We could start a low-priority background thread
		//!! to load a world state into a buffer first, and then
		//!! copy it over the live instance when ready...

	#ifndef DISABLE_SNAPSHOT_COMPRESSION
		ifstream file(fname, ios::binary);
		if (!file || file.bad()) {
			print_error(); return false;
		}

		// Read the whole file into memory:
		stringstream in(ios::in|ios::out|ios::binary);
		in << file.rdbuf();
		if (!in || in.bad()) { print_error(); return false; }

		// Decompress in-place (i.e. replacing the original compr. data; in one go, in-memory... <- !!IMPROVE)
		if (!in.view().starts_with("MODEL") /*!! or !...<hopefully uniform various post-0.1 versions> !!*/) { // Compressed
			try { // Mainly (or only?) for bad_alloc due to garbled data.
				auto cbuf_size = in.view().size();
				auto cbuf = in.view().data();
				auto data_size = ZSTD_getFrameContentSize(cbuf, cbuf_size);
				auto data = make_unique_for_overwrite<char[]>(data_size);
				[[maybe_unused]] auto dsize = ZSTD_decompress(data.get(), data_size, cbuf, cbuf_size);
				assert(dsize == data_size);

				//!!Only in c++26: in.str(string_view((char*)data.get(), data_size)); // or: reset, then: in.write(data.get(), data_size);
				in.seekp(0, in.beg); // out
				in.write(data.get(), data_size);
				in.seekg(0, in.beg); // in
			} catch(...) {
				print_error("- ERROR: Couldn't decompress \""s + fname + "\": unknown or damaged file"s);
				return false;
			}
		} // Compressed?

		if (!in || in.bad()) { print_error(); return false; }

		if (!WorldT::load(in, world_in)) {
			print_error(); return false;
		}
		assert(in && !in.bad());
	#else //DISABLE_SNAPSHOT_COMPRESSION
	#error DISABLE_SNAPSHOT_COMPRESSION is NOT properly implemented! (It should probably be removed instead; compression can already be disabled at runtime!)
	/*
		ifstream in(fname, ios::binary);
		if (!in || in.bad()) { print_error(); return false; }

		//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
		//!!in >> BUILD_ID...

		if (!WorldT::load(in, snapshot)) {
			print_error(); return false;
		}
		assert(in && !in.bad());
	*/
	#endif //DISABLE_SNAPSHOT_COMPRESSION

		Note("World state loaded from \"" + fname + "\".");
		return true;
	} // load































/*!!
	// 1. RESTORE THE WORLD API
	// The App sees "GetEntities()" again, typed correctly.
	SpanLite<EntityT> entities() {
		return { (EntityT*)SimApp::entity_data(0), SimApp::entity_count() };
	}

	EntityT& entity(EntityID id) { return entities()[id]; }

	// 2. RESTORE ADD_ENTITY
	EntityID add_entity(EntityT&& temp) {
		EntityID id = AllocEntitySlot();
		// Placement new into the raw blob
		new (&entity(id)) EntityT(std::move(temp));
		return id;
	}

	// 3. RESTORE PLAYERS
	// The vectors live here now, where the type is known.
	std::vector<PlayerT> players;

	// 4. IMPLEMENT INFRASTRUCTURE
	void SerializeWorld(Stream& s) override {
		// Write m_entityBlob directly...
	}
!!*/
};


} // namespace Szim

#endif // _SADFM987HUYY09786UHYTIUHGJM9587CUV_
