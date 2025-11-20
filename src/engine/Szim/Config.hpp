//!! MOVE THIS TO A "generic" or "foundations" space under the Engine! -> #632

#ifndef _298745SLKWERKJUEIUYCUIUIE12346_
#define _298745SLKWERKJUEIUYCUIUIE12346_

#include <string>
	using namespace std::string_literals; // Ugh: just using ...::operator""s won't even compile...
#include <string_view>
#include <functional> //! For defining `Callback` (see below)...

namespace Szim {

class Config_impl; // friend-ing it inside the class isn't enough for GCC! :-o

class Config
{
public:
	//--------------------------------------------------------------------
	using Callback = std::function<void(Config&)>;
//!!?? Could this somehow cover most of the cases, without the <functional> burden?
//!!??	using Callback = void(*)(Config&);
//!!   Would require ditching the `... = [](auto&&...) {}` generic default lambda in the ctor though, even just to compile...
//!!   But I'm not even sure about the current intended semantics here any more (thanks, past me!... ;-p ) -> #628!

	//--------------------------------------------------------------------
	// Load...
	//--------------------------------------------------------------------
	//!! Support comfy cfg "tagging" (profile selection), at least by filename! (And then rename the cfg_path param to _tag or _uri or sg.)

	// Calls select(), throws on error (if cfg_path is not empty, but couldn't be loaded)
	Config(std::string_view cfg_path,    //!!?? = "" to allow default empty init with a later select()?
	       const Config* base = nullptr, // Optional chained "base" cfg. instance for defaults
	       std::string defaults = "",    // Last-ditch built-in defaults (as raw cfg. text)
	       const Callback& post_load = [](auto&&...) {});

	~Config();

	// Selects .defaults if cfg_path is empty:
	bool select(std::string_view cfg_path, bool can_throw = false,
		const Callback& post_load = [](auto&&...) {}); //!C++23: Alas, `Callback` still can't be auto (not even with a non-lambda default!) :-/

	// Loads a named configuration, from a place specific to the currently integrated backend (selected at compile-time in Config.cpp):
	// Implemented by the backend implementation.
	// (This would be a pure virtual of the `Config_impl` interface base, if this was a real PIMPL pattern!... ;) -> #628!)
	bool _load(std::string_view cfg_path, bool can_throw = false) noexcept;

	//--------------------------------------------------------------------
	// Typed getters...
	//--------------------------------------------------------------------
	const char* get(std::string_view name, const char* def = "")   const noexcept; // 'name' can also be "section/name" //!!?? Only for this one, or is this just a misplaced comment?!
	std::string get(std::string_view name, const std::string& def) const noexcept { return get(name, def.c_str()); }
	//!! Jesuschrist (C++ again), get("...", "default") won't select the string version above,
	//!! but rather some of the numbers below, without the const char* variant!... :-/
	bool        get(std::string_view name, bool def)     const noexcept;
	int         get(std::string_view name, int def)      const noexcept;
	unsigned    get(std::string_view name, unsigned def) const noexcept;
	float       get(std::string_view name, float def)    const noexcept;
	double      get(std::string_view name, double def)   const noexcept;
	//! Alas, only one of these can meaningfully omit the 2nd arg to not be ambiguous!

	//--------------------------------------------------------------------
	// Misc. queries...
	//--------------------------------------------------------------------
	std::string current()   const noexcept; // empty() means using .defaults
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
		// to load, and that fails, these defaults WILL NOT be used,
		// but errors willbe reported.
/*!! NOT YET:
		// Will also be used as a fallback for missing keys, when queried
		// with `get<type>("key")` instead of `get("key", defval)`.
!!*/

private:
	friend class Config_impl;
	Config_impl* _impl;
	const Config* _base = nullptr;     // Optional chained fallback instance for queries

	std::string _current_config; // path (string)
	std::string _cfg_base_path; //!!TBD: append trailing (back)slash? BEWARE:
	                           //!!It's just a quirk of Windows, Unix etc.; IT'S NOT PART OF THE PATH!!
}; // class Config

} // namespace Szim

#endif // _298745SLKWERKJUEIUYCUIUIE12346_
