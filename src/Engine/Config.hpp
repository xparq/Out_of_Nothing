#ifndef _298745SLKWERKJUEIUYCUIUIE12346_
#define _298745SLKWERKJUEIUYCUIUIE12346_

#include <string>
	using namespace std::string_literals; // Ugh: just using ...::operator""s won't even compile...
#include <string_view>
#include <functional>

namespace Szim {

class Config_impl; // friend-ing it inside the class isn't enough for GCC! :-o

class Config
{
public:
	//--------------------------------------------------------------------
	using Callback = std::function<void(Config&)>;

	//--------------------------------------------------------------------
	// Load/parse...
	//--------------------------------------------------------------------
	//!! Support comfy cfg tagging, at least by filename!

	// Calls select(), throws on error (only if cfg_path is not empty, but not found): //!! That error-throw condition is NOT GUARANTEED/ENFORCED! -> #628
	Config(std::string_view cfg_path,  //!!?? = "" to allow default empty init with a later select()?
	       Config* base = nullptr,     // Chained "base" cfg. instance for defaults
	       std::string defaults = "",  // Final internal fallback defaults (as raw cfg. text)
	       const Callback& post_load = [](auto&&...) {});

	~Config();

	// Selects .defaults if cfg_path is empty:
	bool select(std::string_view cfg_path, bool can_throw = false,
		const Callback& post_load = [](auto&&...) {});

	//--------------------------------------------------------------------
	// Typed getters...
	//--------------------------------------------------------------------
	std::string get(std::string_view name, const char* def = "") noexcept; // 'name' can also be "section/name"
	std::string get(std::string_view name, const std::string& def) noexcept { return get(name, def.c_str()); }
	//!! Jesuschrist (C++ again), get("...", "default") won't select the string version above,
	//!! but rather some of the numbers below, without the const char* variant!... :-/
	bool        get(std::string_view name, bool def) noexcept;
	int         get(std::string_view name, int def) noexcept;
	unsigned    get(std::string_view name, unsigned def) noexcept;
	float       get(std::string_view name, float def) noexcept;
	double      get(std::string_view name, double def) noexcept;
	//! Alas, only one of these can meaningfully omit the 2nd arg to not be ambiguous!

	//--------------------------------------------------------------------
	// Misc. helpers...
	//--------------------------------------------------------------------
	std::string current() const noexcept; // empty() means using .defaults
	std::string base_path() const noexcept { return _cfg_base_path; }

//----------------------------------------------------------------------------
// Data...
//----------------------------------------------------------------------------

	std::string defaults;
		// In-memory config "file" for built-in defaults
		//
		// If the ctor. or select("cfg", ...) is called with an empty
		// path, this "file" will be "loaded" and parsed.
		//
		// OTOH, if there was an explicit non-empty path for a config
		// to load, and that fails, the error is not masked by using
		// the defaults instead!
/*!! NOT YET:
		// Will also be used as a fallback for missing keys, when queried
		// with `get<type>("key")` instead of `get("key", defval)`.
!!*/

private:
	friend class Config_impl;
	Config_impl* _impl;
	Config* _base = nullptr;     // Optional chained "fallback" instance for queries

	std::string _current_config; // path (string)
	std::string _cfg_base_path; //!!TBD: append trailing (back)slash? BEWARE:
	                           //!!It's just a quirk of Windows, Unix etc.; IT'S NOT PART OF THE PATH!!
}; // class Config

} // namespace Szim

#endif // _298745SLKWERKJUEIUYCUIUIE12346_
