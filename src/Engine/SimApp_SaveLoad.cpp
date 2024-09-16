#include "SimApp.hpp"

#include <string>
	using std::string, std::to_string;
	using namespace std::string_literals;
//#include <string_view>
//	using std::string_view;
#include "sz/sys/fs.hh"
	using sz::prefix_if_rel;
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#include <iostream>
	using std::cerr, std::cout, std::endl;

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

	string fname = sz::prefix_if_rel(cfg.session_dir, unsanitized_filename);

	auto print_error = [&fname](string alt_msg = "<unset>") {
		if (alt_msg != "<unset>") cerr << alt_msg << (alt_msg.empty() ? "":"\n"); // Allow "" for no custom msg!
		else cerr << "- ERROR: Couldn't save snapshot to file \"" << fname << "\"" << '\n';
		perror("");
	};

	Model::World snapshot = world();
	//!! OK, now we could start a low-priority background thread to actually save the snapshot...

	//!! Note: perror("") may just print "No error" even if the stream is in failure mode! :-/

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
		ofstream out(fname, ios::binary);
		if (!out || out.bad()) {  print_error(); return false; }

		//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
		//!!out << "BUILD_ID = " << ::BUILD_ID << endl;
		if (!snapshot.save(out)) {
			print_error();
			return false;
		}
		assert(out && !out.bad());
	} // Compressed?

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

	if (!Model::World::load(in, &snapshot)) {
		print_error(); return false;
	}
	assert(in && !in.bad());
#else //DISABLE_SNAPSHOT_COMPRESSION
/*
	ifstream in(fname, ios::binary);
	if (!in || in.bad()) { print_error(); return false; }

	//!!Redesign this proc. so that such customizations can be handled by a descendant's save_...() override:
	//!!in >> BUILD_ID...

	if (!Model::World::load(in, &snapshot)) {
		print_error(); return false;
	}
	assert(in && !in.bad());
*/
#endif //DISABLE_SNAPSHOT_COMPRESSION

	set_world(snapshot);

	cerr << "World state loaded from \"" << fname << "\".\n";
	return true;
} // load


} // namespace Szim

