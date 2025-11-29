// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
//!!#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "app/Model/World.hpp"

//!! Shrink even this dependency: don't include the entire fucking SimApp,
//!! only its modelling-related services that are actually used:
//#include "Szim/SimApp.hpp"

#include "Szim/diag/Error.hpp"
#include "Szim/diag/Log.hpp"

#include "extern/semver.hpp"
#include "extern/flatbuffers/flexbuffers.h" // Schemaless self-descriptive format

#include <cassert>
#include <fstream>
	using std::ofstream, std::ifstream;
#include <iomanip>
//	using std::quoted;
//!!#include <optional> //!!NOT YET! Too cumbersome for too little gain.
//!!	using std::optional, std::nullopt;
#include <map>
#include <string>
	using std::stoi, std::stof;
//#include <cstdlib> // strtof
#include <utility>
	using std::move;

//#include "extern/iprof/iprof.hpp"


namespace OON::Model {

using namespace std;
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

	out << "drag = " << props.friction << '\n';
	out << "interactions = " << props.interact_n2n << '\n';
	if (saved_version >= semver::version("0.1.0"))
		{ out << "gravity_mode = " << (unsigned)props.gravity_mode << '\n'; }
	if (saved_version >= semver::version("0.1.2"))
		{ out << "gravity_strength = " << props.gravity << '\n'; }

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
	map<string, string> new_props;

	try {
		// Read the global world properties...
		for (string name, eq, val; in && !in.bad()
			&& (in >> name >> eq >> std::quoted(val)) && eq == "=";) {
			new_props[name] = val;
		}
		LOGD << "LOADED metadata & World params: "; for (auto& [n, v] : new_props) LOGD << " - " << n << ": " << v;
	} catch (...) {
		Error("Failed to read world data!");
		return false;
	}

	const semver::version runtime_version(Model::VERSION);
	const semver::version loaded_version(new_props["MODEL_VERSION"]);

	//!! Verify a prev. save assuming a locked state, so the world hasn't changed since.
	//!! This might be a very stupid idea actually...
	//!!?? [Future me:] WTF did I even mean by this above?!?!
	if (loaded_version > runtime_version) {
		Error("Unsupported snapshot version:" + new_props["MODEL_VERSION"]);
		return false;
	}
	if (loaded_version != runtime_version) {
		Note("Loading a version (" + loaded_version.to_string() + ") older than the runtime ("
		    + runtime_version.to_string() + ").\n  Consider resaving in the new format to avoid obsolescence!");
	}

	/*
	if (stoul(new_props["interactions"]) > 1) {
		Error("Inconsistent snapshot data! (`interactions` is not bool?!)");
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
		++_prop_ndx_; w_new.props.friction     = stof(new_props["drag"]);
		++_prop_ndx_; w_new.props.interact_n2n = stof(new_props["interactions"]);
		if (loaded_version >= semver::version("0.1.0"))
			{ ++_prop_ndx_; w_new.props.gravity_mode = (GravityMode)stoul(new_props["gravity_mode"]);
LOGD << "World::load: gravity_mode = " << (unsigned)w_new.props.gravity_mode;
			}
		if (loaded_version >= semver::version("0.1.2")) // G was saved incorrectly (cast to unsigned) in 0.1.1; ignore that
			{ ++_prop_ndx_; w_new.props.gravity = stof(new_props["gravity_strength"]);
//LOGD << "gravity strength after load: " << w_new.props.gravity;
			}
	} catch (...) {
		Error("Invalid (type of) property #" + to_string(_prop_ndx_) + " in the loaded snapshot.");
		return false;
	}

	// Load the objects, too...
	size_t obj_count = stoi(new_props["objects"].c_str()); //!! stoi may throw!!!
	w_new.bodies.reserve(obj_count);
//LOGD << obj_count << " ~ " << w_new.bodies.size();
	for (size_t n = 0; in && !in.bad() && n < obj_count; ++n) {
		size_t ndx; char c;
		in >> ndx >> c;
//LOGD << "char after the index: '"<< c <<"'";
		assert(n == ndx);

		Entity template_obj;
		if (!Entity::load(in, &template_obj)) {
			Error("Loading entity #" + to_string(n) + " failed!");
			return false;
		}
		w_new.add_body(std::move(template_obj));
	}

	return true;
} // load

} // namespace OON::Model
