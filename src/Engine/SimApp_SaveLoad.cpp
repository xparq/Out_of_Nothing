#define SAVE_COMPRESSED //!! Should be runtime-controllable (e.g. disabled for testing etc.)!
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

#ifdef SAVE_COMPRESSED
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


} // namespace Szim

