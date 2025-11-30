#include "SimApp.hpp"

#include "diag/Log.hpp"
#include "diag/Error.hpp"

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
bool SimApp::save_snapshot(const char* unsanitized_filename, SaveOpt flags)
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

	OON::Model::World snapshot = world(); //!! Oof...
	//!! OK, now we could start a low-priority background thread to actually save the snapshot...

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
bool SimApp::load_snapshot(const char* unsanitized_filename)
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

	//!! Oof... :-/
	std::unique_ptr<OON::Model::World> snapshot // The input buffer
		= create_world();

#ifndef DISABLE_SNAPSHOT_COMPRESSION
	ifstream file(fname, ios::binary);
	if (!file || file.bad()) {
		print_error(); return false;
	}

	// Read the whole file into memory:
	stringstream in(ios::in|ios::out|ios::binary);
	in << file.rdbuf();
	if (!in || in.bad()) { print_error(); return false; }

	// Decompress it "in-place" (i.e. replacing the original compr. data; in one go, in-memory... <- !!IMPROVE)
	if (!in.view().starts_with("MODEL") /*!! or !...<hopefully uniform various post-0.1 versions> !!*/) { // Compressed
		try { // Mainly (or only?) for bad_alloc due to garbled data.
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
		} catch(...) {
			print_error("- ERROR: Couldn't decompress \""s + fname + "\": unknown or damaged file"s);
			return false;
		}
	} // Compressed?

	if (!in || in.bad()) { print_error(); return false; }

	if (!OON::Model::World::load(in, snapshot.get())) {
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

	if (!OON::Model::World::load(in, snapshot)) {
		print_error(); return false;
	}
	assert(in && !in.bad());
*/
#endif //DISABLE_SNAPSHOT_COMPRESSION

	set_world(snapshot);

	Note("World state loaded from \"" + fname + "\".");
	return true;
} // load


} // namespace Szim

