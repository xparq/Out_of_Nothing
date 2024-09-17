#include "Config.hpp"

#include "extern/toml++/sz-toml.hpp" // Proxy header for a custom setup of TOML++

#include <stdexcept>
#include <cassert>
#include <iostream> // For config load/parse errors, when exceptions are disallowed

//============================================================================
#include "sz/sys/fs.hh" // dirname, basename
#include "sz/DBG.hh"

static auto split(std::string_view name)
{
	struct { std::string section, prop; } result;
	// "Abuse" sz::dirname()/basename() to extract (hierarchical) section name:
	result.section = sz::dirname(name);
	//!! If the section name is identical to 'name', that means no section:
	if (result.section == name) result.section = "";
	result.prop = sz::basename(name);
//DBG "cfg get: section: " << result.section;
//DBG "cfg get: param: " << result.prop;
	return result;
}


//============================================================================
namespace Szim {

using namespace std;

//----------------------------------------------------------------------------
class Config_impl
{
friend class Config;

	Config& _cfgi;
	toml::table _config;

	Config_impl(Config& ifc) : _cfgi(ifc) {}
	bool select(string_view cfg_path, bool can_throw, const Config::Callback& post_load);

	// Typed getters...

//	// Special-casing const char* -> string (as the template would incorrectly return const char* if def is that!):
//	string _get(string_view propname, const char* def) { return _get(propname, string(def)); }

	template <typename T>
	T _get(string_view propname, T def) {
		auto [section, name] = split(propname); // Splits at the *last* / (if any)
		return section.empty() ? _config[name].value_or(def) //!!?? Why not the same chained fallback as below?! :-o
		                       : _config[section][name].value_or(_cfgi._base ? _cfgi._base->get(propname, def) : def);
	}

//!!??string   Config::get(string_view prop, const char* def) noexcept { return _impl->_get(prop, string(def)); }
	//!!?? Why is this weird messing with the default type?! If it's specific to TOML++, then it should
	//!!?? be dealt with in its adapter impl.!

};

bool Config_impl::select(std::string_view cfg_path, bool can_throw, const Config::Callback& post_load)
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
		else { std::cerr << errmsg; return false; }
	}
	_config = result.table(); //!! Will not compile with exceptions enabled (I guess)!

	_cfgi._current_config = cfg_path;
	_cfgi._cfg_base_path = sz::dirname(cfg_path);

	// Call the user callback (NOOP by default)
	post_load(_cfgi);
	return true;
}

} // namespace Szim