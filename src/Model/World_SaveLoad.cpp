// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
//!!#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "Model/World.hpp"

//!! Shrink even this dependency: don't include the entire fucking SimApp,
//!! only its modelling-related services that are actually used:
//#include "Engine/SimApp.hpp"

#include "extern/semver.hpp"
#include "extern/flatbuffers/flexbuffers.h" // Schemaless self-descriptive format

#include <cassert>
#include <fstream>
	using std::ofstream, std::ifstream;
#include <iomanip>
//	using std::quoted;
#include <iostream>
	using std::cerr;
//!!NOT YET! Too cumbersome for the trivial alternative.
//!!#include <optional>
//!!	using std::optional, std::nullopt;
#include <map>
#include <string>
	using std::stoi, std::stof;
//#include <cstdlib> // strtof
#include <utility>
	using std::move;

//#include "extern/iprof/iprof.hpp"


namespace Model {

using namespace std;
using namespace Szim;
using namespace Math;

//----------------------------------------------------------------------------
bool World::save(std::ostream& out, [[maybe_unused]] const char* version/* = nullptr*/)
{
//!! For reg. testing:
//!!	version = "0.0.1";
	semver::version saved_version(version ? version : Model::VERSION);

	//!! Dummy compilation smoke test only yet:
	flexbuffers::Builder fbb;

	out << "MODEL_VERSION = " << saved_version << '\n';

	out << "drag = " << friction << '\n';
	out << "interactions = " << _interact_all << '\n';
	if (saved_version >= semver::version("0.1.0"))
		{ out << "gravity_mode = " << (unsigned)gravity_mode << '\n'; }
	if (saved_version >= semver::version("0.1.2"))
		{ out << "gravity_strength = " << gravity << '\n'; }

//!!This should go to session files, along with other app-level data (like view scale etc.)!
//!!	if (saved_version >= semver::version("0.1.3"))
//!!		{ out << "time_scale = " << time.scale << '\n'; }

	out << "objects = " << bodies.size() << '\n'; // Saving for verification + preallocation on load!
	out << "- - -" << '\n'; //!! Mandatory 3-token separator to not break the idiotic loader! :)
	for (size_t ndx = 0; ndx < bodies.size(); ++ndx) {
		//!!Doesn't work, in>> struggles with the binary data, can't deal with it basically...
		out << ndx << " : " // not "=" in order to assist load() a bit...
		                    // Also, the space is needed before the ':' to allow reading the index (even into an int)!
		<< "\"";
			if (!bodies[ndx]->save(out)) {
				return false;
			}
		out << "\"" << '\n'; //! For *some* readability. (Whitespace will be skipped after the bin chunk.)
	}

	return out && !out.bad();
} // save

//----------------------------------------------------------------------------
//!!optional<World> World::load(std::istream& in) // static (factory)
//!!{
//!!	optional<World> w0;
//!!	return nullopt; //!! w0;
//!!}
/*static*/ bool World::load(std::istream& in, World* result)
{
	map<string, string> props;

	try {
		// Read the global world properties...
		for (string name, eq, val; in && !in.bad()
			&& (in >> name >> eq >> std::quoted(val)) && eq == "=";) {
			props[name] = val;
		}
	//cerr << "DBG> LOADED:\n"; for (auto& [n, v] : props) cerr << n << ": " << v << endl;
	} catch (...) {
		cerr << "- ERROR: Failed to read world data!\n";
		return false;
	}

	const semver::version runtime_version(Model::VERSION);
	const semver::version loaded_version(props["MODEL_VERSION"]);

	//!! Verify a prev. save assuming a locked state, so the world hasn't changed since.
	//!! This might be a very stupid idea actually...
	if (loaded_version > runtime_version) {
		cerr << "- ERROR: Unsupported snapshot version \"" << props["MODEL_VERSION"] << "\"\n";
		return false;
	}
	if (loaded_version != runtime_version) {
		cerr << "- NOTE: Loading a version ("<< loaded_version <<") older than the runtime ("
			<< runtime_version <<").\n  Consider resaving in the new format to avoid obsolescence!\n";
	}

	/*
	if (stoul(props["interactions"]) > 1) {
		cerr << "- ERROR: Inconsistent snapshot data (`interactions` is not bool?!)\n";
		return false;
	}*/
	//!!
	//!!...
	//!!
	//!!Phew, done checking... :))

	if (!result) {
		return false; //!! VERIFY NOT IMPLEMENTED YET!
	}

	World& w_new = *result;

	unsigned _prop_ndx_ = 0;
	try { // stof & friends are throw-happy
		++_prop_ndx_; w_new.friction = stof(props["drag"]);
		++_prop_ndx_; w_new._interact_all = stof(props["interactions"]);
		if (loaded_version >= semver::version("0.1.0"))
			{ ++_prop_ndx_; w_new.gravity_mode = (GravityMode)stoul(props["gravity_mode"]);
cerr << "DBG> World::load: gravity_mode = " << (unsigned)w_new.gravity_mode << '\n';
			}
		if (loaded_version >= semver::version("0.1.2")) // G was saved incorrectly (cast to unsigned) in 0.1.1; ignore that
			{ ++_prop_ndx_; w_new.gravity = stof(props["gravity_strength"]);
//cerr << "DBG> gravity strength after load: " << w_new.gravity << '\n';
			}
	} catch (...) {
		cerr << "- ERROR: Invalid (type of) property #"<<_prop_ndx_<<" in the loaded snapshot.\n";
		return false;
	}

	// Load the objects, too...
	auto obj_count = stoi(props["objects"].c_str()); //!! stoi may throw!!!
	w_new.bodies.reserve(obj_count);
//cerr << obj_count << " ~ " << w_new.bodies.size() << endl;
	for (size_t n = 0; in && !in.bad() && n < obj_count; ++n) {
		size_t ndx; char c;
		in >> ndx >> c;
//cerr << "char after the index: '"<< c <<"'" <<endl;
		assert(n == ndx);

		Body template_obj;
		if (!Body::load(in, &template_obj)) {
			cerr << "- ERROR: Loading entity #"<< n <<" failed!\n";
			return false;
		}
		w_new.add_body(std::move(template_obj));
	}

	return true;
} // load

} // namespace Model