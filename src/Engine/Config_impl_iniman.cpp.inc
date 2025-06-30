//============================================================================
// API
#include "Config.hpp"

//============================================================================
// Backend - IniMan adapter 

#define INIMAN_ENABLE_GET_STD_STRING //! Required for supporting the string-returning get() API of Enginge/Config.hpp!
#include "extern/iniman/iniman.cc"

#include "sz/sys/fs.hh" // dirname, basename
#include "sz/str.hh"    // to_bool

//#include <cassert>
#include "diag/Log.hpp"
//#include "diag/Error.hpp"
//#undef NDEBUG // Just for quick ad-hoc debug dumps, of course...
//#include "sz/DBG.hh"


//============================================================================
namespace {

auto split(std::string_view name)
//
// Split "section name/prop name" into { "section name"s, "prop name"s }
// or just "prop name" to { "", "prop name"s }
//
//!! This is a pretty generic/common task; should be in szlib already!
//!! (But with a less flamboyant implementation then, perhaps. ;) )
{
	struct { std::string section, prop; } result;
	// "Abuse" sz::dirname()/basename() to extract the (hierarchical) section name, if any:
	result.section = sz::dirname(name);
	// If section name == name, then no section:
	if (result.section == name) result.section = "";
	// The propertry name is the tail-end:
	result.prop = sz::basename(name);
//DBG "cfg get: section: " << result.section;
//DBG "cfg get: param: " << result.prop;
	return result;
}

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
	IniMan  _db;

	Config_impl(Config& ifc)
	:	_cfgi(ifc),
		_db(IniMan::Config{
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

	bool _load(std::string_view cfg_path) noexcept
	{
	//DBG "Loading config from " << (cfg_path ? cfg_path : "built-in defaults") << "...";
		string fname(cfg_path);
		auto result = false;
		if (cfg_path.empty()) {
LOGW << "NO cfg_path — loading config from built-in defaults!";
			_db.cfg.LOAD_FROM_STRING = true;
			result = _db.load(_cfgi.defaults.c_str());
		} else {
LOGD << "Loading config from " << fname;
			_db.cfg.LOAD_FROM_STRING = false;
			result = _db.load(fname.c_str());
LOGD << "- load() result: " << result;
LOGD << "- testing cfg read: asset_dir == " << _get("asset_dir", "WTF?!");
		}
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

	// ...This is not what Config.hpp says:
	//		Error("Failed to load configuration from '" + string(cfg_path) + "' ");
	//		return false;
	// ...And this is way too heavy-handed here:
	//		Abort("Failed to load configuration from '" + string(cfg_path) + "' ");
	// ...This might be OK, but then it should be done by the generic Config itself:

		}

		return result;
	}

	//
	// Typed getters...
	//
	template <typename T> //!!??, typename = enable_if_t<!is_same_v<T, string>>>
	T _get(string_view propname, T def) {
		auto [section, name] = split(propname); // Splits at the *last* / (if any)
		string section_s(section);
		string key_s(name);
		return section.empty() ? _db.get(key_s.c_str(), def) //!!?? Why not the same chained fallback as below?! :-o
		                       : _db.get(section_s.c_str(), key_s.c_str(), _cfgi._base ? _cfgi._base->get(propname, def) : def);
	}

	// Special-casing bool:
	bool _get(string_view propname, bool def) {
//DBG "BOOL: " << propname;
		string boolstr = _get(propname, string(1, static_cast<char>(def)));
		return sz::to_bool(boolstr);
	}

}; // class Config_impl


} // namespace Szim
