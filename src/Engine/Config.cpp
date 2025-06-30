//!!
//!! The current config subsystem design is too (subtly) convoluted for not documenting it at all... -> #628!
//!!

#include "Config.hpp"

//#include "Config_impl_TOML++.cpp.inc"
// My simple INI reader takes 5-6s(!!!) (and ~120K) less to compile,
// and it doesn't require quoting [main/sub] section names...
#include "Config_impl_iniman.cpp.inc"

//#include "diag/Log.hpp"
//#include "diag/Error.hpp"

//============================================================================
// Generic Config impl. -- most of it depends on Config_impl already defined!
//============================================================================

namespace Szim {

using namespace std;

//----------------------------------------------------------------------------
Config::Config(std::string_view cfg_path, Config* base, std::string defaults, const Callback& post_load)
	: _base(base)
	, defaults(defaults)
{
	_impl = new Config_impl(*this);

	select(cfg_path, true, post_load); // `true` -> make it throw, when called from the ctor!
}

//----------------------------------------------------------------------------
Config::~Config()
{
	delete _impl;
}

//----------------------------------------------------------------------------
bool Config::select(std::string_view cfg_path, bool can_throw, const Callback& post_load)
{
	if (cfg_path.empty())
	{
		// Use built-in defaults...
	}

	if (_impl->_load(cfg_path))
	{ // Success...
		_current_config = cfg_path;
		_cfg_base_path = sz::dirname(cfg_path);

		// Call the user callback (NOOP by default)
		post_load(*this);
		return true;
	}

	// Fail...
	if (can_throw)
		throw std::runtime_error("Failed to load configuration from \"" + string(cfg_path) + "\"");
	else
		return false;
}

//----------------------------------------------------------------------------
std::string Config::current() const noexcept
{
	return _current_config; //!! But... implement tagging!... (That's why this is not in the header.)
}

//----------------------------------------------------------------------------
// Supported typed getters
//----------------------------------------------------------------------------
//!!...string   Config::get(string_view prop, const char* def) noexcept { return _impl->_get(prop, string(def)); }
const char*   Config::get(string_view prop, const char* def) noexcept { return _impl->_get(prop, def); }
bool     Config::get(string_view prop, bool def)        noexcept { return _impl->_get(prop, def); }
int      Config::get(string_view prop, int def)         noexcept { return _impl->_get(prop, def); }
unsigned Config::get(string_view prop, unsigned def)    noexcept { return _impl->_get(prop, def); }
float    Config::get(string_view prop, float def)       noexcept { return _impl->_get(prop, def); }
double   Config::get(string_view prop, double def)      noexcept { return _impl->_get(prop, def); }

} // namespace Szim