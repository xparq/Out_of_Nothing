#ifndef _SLKWERKJUEIUYCUIUIE12346_
#define _SLKWERKJUEIUYCUIUIE12346_

#include <string>
	using namespace std::string_literals; // Damn... Just ...::operator""s did't compile, WTF?!
#include <functional>

namespace Szim {

struct Config
{
	//--------------------------------------------------------------------
	static std::string Defaults;
		// Default in-memory config "file"
		//
		// If the ctor. of select("cfg", ...) is called with an empty
		// path, this "file" will be "loaded" and parsed.
/*!! NOT YET:
		// Will be used as a fallback for missing keys, when queried
		// with `get<type>("key")` instead of `get("key", defval)`,
		// where the type of defval decides the return type.
!!*/
		// Client code is free to set this to whatever, whenever.
		// It will only be used by a subsequent select(), in any of
		// the cfg. instances (!!OF WHICH THERE SHOULD BE ONLY ONE NOW!).

	//--------------------------------------------------------------------
	using CALLBACK = std::function<void(Config*)>;

	//----------------------------------------------------------------------------
	// Load/parse...
	//----------------------------------------------------------------------------
	//!! Support comfy cfg tagging, at least by filename!

	// Calls select(), throws on error:
	Config(const std::string& cfg_path,
		const CALLBACK& post_load = [](auto&&...) {});

	// Selects ::Defaults if cfg_path is empty:
 	bool select(const std::string& cfg_path, bool can_throw = false,
		const CALLBACK& post_load = [](auto&&...) {});

	//----------------------------------------------------------------------------
	// Typed getters...
	//----------------------------------------------------------------------------
	std::string get(const std::string& name, const char* def = ""); // 'name' can also be "section/name"
	std::string get(const std::string& name, const std::string& def) { return get(name, def.c_str()); }
	//!! Jesuschrist (C++ again), get("...", "default") won't select the string version above,
	//!! but rather some of the numbers below, without the const char* variant!... :-o
	int         get(const std::string& name, int def);
	float       get(const std::string& name, float def);
	bool        get(const std::string& name, bool def);
	//! Alas, only one of these can be meaningfully omit the 2nd arg, to not be ambiguous!

	//----------------------------------------------------------------------------
	// Misc. helpers...
	//----------------------------------------------------------------------------
	std::string current() const; // empty() means using ::Defaults
	std::string base_path() const { return _cfg_base_path; }

private:
	std::string _current_config; // path (string)
	std::string _cfg_base_path; //!!TBD: append trailing (back)slash? BEWARE:
	                           //!!It's just a quirk of Windows, Unix etc.; IT'S NOT PART OF THE PATH!!
};

} // namespace Szim
#endif // _SLKWERKJUEIUYCUIUIE12346_
