#include "Config.hpp"

#define INIMAN_CFG_GET_STD_STRING
#include "extern/iniman/iniman.cc"

#include <stdexcept>
#include <cassert>
#include <iostream> // For config load/parse errors, when exceptions are disallowed

//============================================================================
#include "sz/sys/fs.hh" // dirname, basename
#include "sz/str.hh"    // to_bool
#undef NDEBUG
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

using ini::IniMan;
using namespace std;

//----------------------------------------------------------------------------
class Config_impl
{
friend class Config;

	Config& _cfgi;
	IniMan _config;

	Config_impl(Config& ifc)
	:	_cfgi(ifc),
		_config(IniMan::Config{
	//		.SUBSTITUTION = false,
	//		.KEEP_UNRESOLVED_REF_AS_IS = true,
	//		.NUM_C_OCTAL = true,
			.SUBST_ENV = true,
			.CONTINUE_ON_SYNTAX_ERRORS = true,
			.GLOBAL_SEARCH_BY_DEFAULT  = true,
		})
	{
		//IniMan::Config::CONSOLE_MESSAGES = false;
	}

	bool select(std::string_view cfg_path, bool can_throw, const Config::Callback& post_load);

	// Typed getters...
	
	template <typename T> //!!??, typename = enable_if_t<!is_same_v<T, string>>>
	T _get(string_view propname, T def) {
		auto [section, name] = split(propname); // Splits at the *last* / (if any)
		string section_s(section);
		string key_s(name);
		return section.empty() ? _config.get(key_s.c_str(), def) //!!?? Why not the same chained fallback as below?! :-o
		                       : _config.get(section_s.c_str(), key_s.c_str(), _cfgi._base ? _cfgi._base->get(propname, def) : def);
	}

	// Special-casing bool:
	bool _get(string_view propname, bool def) {
//DBG "BOOL: " << propname;
		string boolstr = _get(propname, string(1, static_cast<char>(def)));
		return sz::to_bool(boolstr);
	}

};

bool Config_impl::select(string_view cfg_path, [[maybe_unused]] bool can_throw, const Config::Callback& post_load)
{
//DBG "Loading " << cfg_path;
	string fname(cfg_path);
	auto result = /*cfg_path.empty() ?
		_config.load(_cfgi.defaults) : */_config.load(fname.c_str());
	if (!result) {
/*		
		auto& err = (toml::parse_error&)result;
		auto errmsg = "Failed to load config \""s;
		     errmsg += cfg_path;
		     errmsg += "\": \n    ";
		     errmsg += err.description();
		     errmsg += "\n        at line: ";
		     errmsg += std::to_string(err.source().begin.line);
		if (can_throw) throw std::runtime_error(errmsg);
		else { cerr << errmsg; return false; }
*/
		cerr << "- ERROR: Failed to load configuration from '"<< cfg_path <<"' ";
		return false;
	}

	_cfgi._current_config = cfg_path;
	_cfgi._cfg_base_path = sz::dirname(cfg_path);

	// Call the user callback (NOOP by default)
	post_load(_cfgi);
	return true;
}

} // namespace Szim