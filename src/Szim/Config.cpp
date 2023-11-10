#include "Config.hpp"
#include "sz/fs.hh"
#include "extern/toml++/sz-toml.hpp" // Proxy header with custom cfg.

#include <string_view>
#include <string>
#include <stdexcept>
	using std::runtime_error;
#include <cassert>
#include <iostream> // DEBUG ONLY!


using namespace Szim;
using namespace std;


//----------------------------------------------------------------------------
//!!...
static constexpr auto Defaults = R"(
        str = "hello world"

        numbers = [ 1, 2, 3, "four", 5.0 ]
        vegetables = [ "tomato", "onion", "mushroom", "lettuce" ]
        minerals = [ "quartz", "iron", "copper", "diamond" ]

        [animals]
        cats = [ "tiger", "lion", "puma" ]
        birds = [ "macaw", "pigeon", "canary" ]
        fish = [ "salmon", "trout", "carp" ]
)"sv;


static toml::table _config; //!! Mutex it!

//----------------------------------------------------------------------------
static auto split(const string& name)
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
//----------------------------------------------------------------------------
bool Config::select(const string& cfg_path)
{
	// Handle both error modes of Toml++ (albeit I compile it with no-ex. normally):
	toml::parse_result result;
	try { result = toml::parse_file(cfg_path); }
	catch (...) { assert(!result); }
	if (!result) {
		//!!if enabled:
		throw runtime_error("Failed to load config: "s + cfg_path);
		//!!else:
		//!!return false;
	}
	_config = result.table(); //!! Will not compile with exceptions enabled (I guess)!

cerr << "Active cfg: " << cfg_path << '\n';

	//!! Should reset the old config values!
	//!! And apply any preconditions/dependency rules etc...
	_current_config = cfg_path;
	_cfg_base_path = sz::dirname(cfg_path);
	return true;
}

string Config::current_cfg() const
{
	return _current_config; //!! But... tagging, too!...
}


//----------------------------------------------------------------------------
string Config::get(const string& prop, string def)
{
	auto [section, name] = split(prop);
	return _config[section][name].value_or(def);
}

int Config::get(const string& prop, int def)
{
	auto [section, name] = split(prop);
//cerr << "cfg get["<<section<<"]["<<name<<"]: [" << int(_config[section][name].value_or<int64_t>(def)) << "]\n";
	return int(_config[section][name].value_or<int64_t>(def));
}

float Config::get(const std::string& prop, float def)
{
	auto [section, name] = split(prop);
//cerr << "cfg get["<<section<<"]["<<name<<"]: [" << _config[section][name].value_or(def) << "]\n";
	return _config[section][name].value_or(def);
}

bool Config::get(const string& prop, bool def)
{
	auto [section, name] = split(prop);
//cerr << "cfg get["<<section<<"]["<<name<<"]: [" << _config[section][name].value_or<int64_t>(def) << "]\n";
	return _config[section][name].value_or(def);
}


