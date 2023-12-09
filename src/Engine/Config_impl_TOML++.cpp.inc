#include "Config.hpp"

#include "extern/toml++/sz-toml.hpp" // Proxy header for custom TOML cfg.

#include <stdexcept>
#include <cassert>
#include <iostream> // For config load/parse errors, when exceptions are disallowed
using namespace std;

//============================================================================
#include "sz/fs.hh" // dirname, basename

static auto split(string_view name)
{
	struct { string section, prop; } result;
	// "Abuse" sz::dirname()/basename() to extract (hierarchical) section name:
	result.section = sz::dirname(name);
	//!! If the section name is identical to 'name', that means no section:
	if (result.section == name) result.section = "";
	result.prop = sz::basename(name);
//cerr << "cfg get: section: " << result.section << '\n';
//cerr << "cfg get: param: " << result.prop << '\n';
	return result;
}


//============================================================================
namespace Szim {

//----------------------------------------------------------------------------
class Config_impl
{
friend class Config;

	Config& _cfgi;
	toml::table _config;

	Config_impl(Config& ifc) : _cfgi(ifc) {}
	bool select(std::string_view cfg_path, bool can_throw, const Config::CALLBACK& post_load);

	// Typed getters
	template <typename T>
	T _get(string_view propname, T def) {
		auto [section, name] = split(propname);
		return section.empty() ? _config[name].value_or(def)
		                       : _config[section][name].value_or(_cfgi._base ? _cfgi._base->get(propname, def) : def);
	}
};

bool Config_impl::select(std::string_view cfg_path, bool can_throw, const Config::CALLBACK& post_load)
{
	//!! Wishful attempt below to try and unify both the exc. and noexc.
	//!! modes in one control flow...
	//!! ...and it works fine, as long as exceptions are disabled! :)
	toml::parse_result result;
	try {
		result = cfg_path.empty() ?
			toml::parse(_cfgi.defaults) : toml::parse_file(cfg_path);
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
		if (can_throw) throw std::runtime_error(errmsg);
		else { cerr << errmsg; return false; }
	}
	_config = result.table(); //!! Will not compile with exceptions enabled (I guess)!

	_cfgi._current_config = cfg_path;
	_cfgi._cfg_base_path = sz::dirname(cfg_path);

	// Call the user callback (NOOP by default)
	post_load(_cfgi);
	return true;
}

} // namespace Szim