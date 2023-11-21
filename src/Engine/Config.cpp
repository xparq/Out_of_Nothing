#include "Config.hpp"
#include "sz/fs.hh"

#include "extern/toml++/sz-toml.hpp" // Proxy header with custom cfg.

#include <string_view>
#include <string>
#include <stdexcept>
	using std::runtime_error;
#include <cassert>
#include <iostream> // For config load/parse errors, when exceptions are disallowed

using namespace Szim;
using namespace std;

//----------------------------------------------------------------------------
/*static*/ string Config::Defaults = R"()"s;
//! Alas, due to the C++ static init order fiasco, there's no controlled way
//! for the app to simply override this with its own static definition.
//! The earliest it can be reset is the app ctor.'s member init list.

static toml::table _config;
	//!! Get rid of this! :-o
	//!! There was a reason I made it static, but failed to document it, and now
	//!! I have no idea, of course... :-/
	//!! Hopefully it was just a quick hack for the old architecture with a less
	//!! controlled config init sequence, so stuff can at least grab things from...
	//!! Wait a sec, that doen't make any sense... The cfg. instance itself was
	//!! already not-static! Whatever... Just get rid of this! :)
	//!!
	//!! Aaargh!... OK, got it! :) No, it was for a completely different reason:
	//!! this is a simple, hamfisted, but effective kludge to isolate the rest of
	//!! the system from whatever actual config file format/parser is used!
	//!! So... Not so fast with getting rid of it...
	//!!
	//!! (It could still be replaced with a dynamically allocated buffer though,
	//!! by only forward-declaring the object type in the header. And maybe
	//!! using unique_prt, if someone pays for it... But for such a simple
	//!! task there's absolutely no need for that.)

//----------------------------------------------------------------------------
static auto split(string_view name)
{
	struct { string section, prop; } result;
	//!! Abuse sz::dirname()/basename() to extract any section name...
	result.section = sz::dirname(name);
	//!! If the section name is identical to 'name', that means no section:
	if (result.section == name) result.section = "";
	result.prop = sz::basename(name);
//cerr << "cfg get: section: " << result.section << '\n';
//cerr << "cfg get: param: " << result.prop << '\n';
	return result;
}

//============================================================================
Config::Config(std::string_view cfg_path, const CALLBACK& post_load)
{
	select(cfg_path, true, post_load); // Let it throw
}

//----------------------------------------------------------------------------
bool Config::select(std::string_view cfg_path, bool can_throw, const CALLBACK& post_load)
{
	//!! Wishful attempt below to try and unify both the exc. and noexc. modes
	//!! in one control flow...
	//!! However, I've only tried it with exceptions disabled yet! :)
	toml::parse_result result;
	try {
		result = cfg_path.empty() ?
			toml::parse(Defaults) : toml::parse_file(cfg_path);
	} catch (...) {
		assert(!result); //!! This may well be not how TOML++ works with exceptions enabled!
	}
	if (!result) {
		auto& err = (toml::parse_error&)result;
		auto errmsg = "Failed to load config \""s;
		     errmsg += cfg_path;
		     errmsg += "\": \n    ";
		     errmsg += err.description();
		     errmsg += "\n        at line: ";
		     errmsg += std::to_string(err.source().begin.line);
		if (can_throw) throw runtime_error(errmsg);
		else { cerr << errmsg; return false; }
	}
	_config = result.table(); //!! Will not compile with exceptions enabled (I guess)!
	_current_config = cfg_path;
	_cfg_base_path = sz::dirname(cfg_path);

	// Call the user callback (NOOP by default)
	post_load(this);

	return true;
}

//----------------------------------------------------------------------------
string Config::current() const
{
	return _current_config; //!! But... tagging, too!...
}


//----------------------------------------------------------------------------
// Typed getters - impl.
//----------------------------------------------------------------------------

template <typename T>
T _get(string_view propname, T def) {
	auto [section, name] = split(propname);
	return section.empty()
		? _config[name].value_or(def)
		: _config[section][name].value_or(def);
}

//----------------------------------------------------------------------------
string Config::get(string_view prop, const char* def)
{
	return _get(prop, string(def));
}

int Config::get(string_view prop, int def)
{
	return _get(prop, def);
}

float Config::get(string_view prop, float def)
{
	return _get(prop, def);
}

bool Config::get(string_view prop, bool def)
{
	return _get(prop, def);
}
