#define COMPRESSED_SNAPSHOTS //!! Should be runtime-controllable (e.g. disabled for testing etc.)!
                        //!! But the #includes...

#include "SimApp.hpp"

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
#include <iostream>
	using std::cerr, std::cout, std::endl;

#ifdef COMPRESSED_SNAPSHOTS
#   include "extern/zstd/zstd.h"
#   include <sstream>
    using std::ostringstream, std::stringstream;
#   include <memory>
    using std::make_unique_for_overwrite;
#   include <cstddef>
    using std::byte; //!! It's fucked up in C++ tho: a byte[] buffer can't be used for file IO... Excellent.
#endif

namespace Szim {

//----------------------------------------------------------------------------
bool SimApp::save_snapshot(const char* unsanitized_filename)
{
	//!!A kinda alluring abstraction would be SimApp not really having its own state
	//!!(worth saving, beside the model world), leaving all that to descendants...
	//!!But I suspect it's unfounded; at least I can't see the higher principle it
	//!!could be derived from... What I do see, OTOH, is the hassle in the App class
	//!!chain to actually deal with saving/loading all the meta/supplementary state...

	string fname = sz::prefix_if_rel(cfg.session_dir, unsanitized_filename);

	auto print_error = [&fname](string alt_msg = "<unset>") {
		if (alt_msg != "<unset>") cerr << alt_msg << (alt_msg.empty() ? "":"\n"); // Allow "" for no custom msg!
		else cerr << "- ERROR: Couldn't save snapshot to file \"" << fname << "\"" << '\n';
		perror("");
	};

	Model::World snapshot = world();
	//!! OK, now we could start a low-priority background thread to actually save the snapshot...

	//!! Note: perror("") may just print "No error" even if the stream is in failure mode! :-/

#ifdef COMPRESSED_SNAPSHOTS
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

#else // !COMPRESSED_SNAPSHOTS
	ofstream out(fname, ios::binary);
	if (!out || out.bad()) {  print_error(); return false; }

	//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
	//!!out << "BUILD_ID = " << ::BUILD_ID << endl;
	if (!snapshot.save(out)) {
		print_error();
		return false;
	}
#endif // COMPRESSED_SNAPSHOTS

	assert(out && !out.bad());

	cerr << "World state saved to \"" << fname << "\".\n";
	return true;
} // save

//----------------------------------------------------------------------------
bool SimApp::load_snapshot(const char* unsanitized_filename)
{
	string fname = sz::prefix_if_rel(cfg.session_dir, unsanitized_filename);

	auto print_error = [&fname](string alt_msg = "<unset>") {
		if (alt_msg != "<unset>") cerr << alt_msg << (alt_msg.empty() ? "":"\n"); // Allow "" for no custom msg!
		else cerr << "- ERROR: Couldn't load snapshot from file \"" << fname << "\"" << '\n';
		perror("");
	};

	//!! We could start a low-priority background thread
	//!! to load a world state into a buffer first, and then
	//!! copy it over the live instance when ready...

	Model::World snapshot; // The input buffer

#ifdef COMPRESSED_SNAPSHOTS
	ifstream file(fname, ios::binary);
	if (!file || file.bad()) {
		print_error(); return false;
	}

	// Read the whole file into memory:
	stringstream in(ios::in|ios::out|ios::binary);
	in << file.rdbuf();
	if (!in || in.bad()) { print_error(); return false; }

	// Decompress it "in-place" (i.e. replacing the original compr. data; in one go, in-memory... <- !!IMPROVE)
	if (!in.view().starts_with("MODEL") /*!! or !...<hopefully uniform various post-0.1 versions> !!*/) {
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
	}

	if (!in || in.bad()) { print_error(); return false; }

	if (!Model::World::load(in, &snapshot)) {
		print_error(); return false;
	}

	if (!in || in.bad()) { print_error(); return false; } // Should be redundant, but I've become too paranoid to delete it...

#else // !COMPRESSED_SNAPSHOTS
	ifstream in(fname, ios::binary);
	if (!in || in.bad()) { print_error(); return false; }

	//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
	//!!in >> BUILD_ID...

	if (!Model::World::load(in, &snapshot)) {
		print_error(); return false;
	}
#endif // COMPRESSED_SNAPSHOTS

	assert(in && !in.bad());

	set_world(snapshot);

	cerr << "World state loaded from \"" << fname << "\".\n";
	return true;
} // load


} // namespace Szim

