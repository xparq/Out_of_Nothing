//===========================================================================
//
// Simple config. file reader for a Windows INI/INF-like syntax
//
//===========================================================================

#ifndef _INI_D2FIEU5GDUY43F5NG8YRGYU356F_
#define _INI_D2FIEU5GDUY43F5NG8YRGYU356F_

//---------------------------------------------------------------------------
//  Supports multiple sections (marked with [section name] headers); the 
//  first section can be untitled.
//
//  NOTES:
//
//  - Supports `key=value`-style properties (ignoring spaces around the `=`)
//  - End-of-line comments (`#` and `;` by default)
//  - Quoted values (for preserving space or comment chars etc.)
//  - Typed queries for common types (int, long, string).
//  - Substitution: `thing = $ref` will resolve to the value of `ref` as a key
//    or an environment variable (unless disabled).
//  - Keys are not case-sensitive (upper-cased), when queried.
//  - Loads & keeps the entire config file in memory, and works from there.
//  - Returned strings are const char* pointers into that buffer (not copies).
//    So, DON'T destroy or clear() an instance while using its query results!
//  - UTF8-clean, BOM-aware (refuses to load unsupported UTF encodings).
//  - Not 8-bit binary-clean though: ASCII control codes are not guaranteed
//    to be fully preserved. (Binary data should be text-encoded (e.g. base64,
//    uuencode etc.), as usual.)
//


#ifdef INIMAN_CFG_GET_STD_STRING
#include <string>
#endif

#ifndef INIMAN_NAMESPACE
#define INIMAN_NAMESPACE ini
#endif
namespace INIMAN_NAMESPACE {

class IniMan
{
public:
	struct Config
	{
		static inline const unsigned MAX_LINE_SIZE = 1000; // Source line length limit (incl. line term chars + EOS)
			// Sorry, this can't be changed run-time, as it's used to comp.-time reserve local buffers on the stack!

		static inline bool CONSOLE_MESSAGES = true;
			// Also static, but run-time & non-const, so it's available & switchable in any execution context.

//		const char *const COMMENT_PREFIX[3] = { "#", ";", "//" }; //!! NOT YET USED! Hardcoded in __IS_COMMENT!
		bool NUM_C_OCTAL = false;               // 000123 won't be parsed as octal by default
		bool SUBSTITUTION = true;
		bool SUBST_ENV = true;
		bool KEEP_UNRESOLVED_REF_AS_IS = false; // `$dangling` is a literal string value, or an error?
		int  SUBST_CHAIN_LIMIT = 10;
		bool CONTINUE_ON_SYNTAX_ERRORS = false; // Still reported, and bad lines are ignored, if false.
		bool GLOBAL_SEARCH_BY_DEFAULT  = true;  // Otherwise unqualif. search looks in the unnamed section only (if exists)

		//!! Config(); // For validation & error reporting.
		//!! Except... Can't have ctors with desig. init!... :-/
		bool valid() const; //!! So, this needs to be called manually from the IniMan ctors... Great. :-/
	};

public:
	IniMan(const Config& cfg);

	// Load an entire file, or just a section of it:
	IniMan(const char* fnam, const char* section = 0);
	IniMan(const Config& cfg, const char* fnam, const char* section = 0);

	~IniMan();

	// No copying:
	IniMan(const IniMan&) = delete;

	bool valid() const { return cfg.valid() && buffer_ != 0 /*!!?? && *buffer_ ??!!*/; static_assert(sizeof(int) >= 4, "The int size must be >= 32 bits!"); };
	void clear();

	bool load(const char* fnam, const char* section = 0)
	{
		clear();
		return !!__load(fnam, section);
	}

	//-------------------------------------------------------------------
	// Getters...
	//
	// Return `def` if the item couldn't be found or parsed.
	//
	//-------------------------------------------------------------------
	//
	// Find in a named section...
	//
	const char*   get(const char* section, const char* name, const char* def) const { return __get_raw(section, name, def); }
#ifdef INIMAN_CFG_GET_STD_STRING
	std::string   get(const char* section, const char* name, std::string def) const { return get(section, name, def.c_str()); }
#endif
	int           get(const char* section, const char* name, int def) const;
	unsigned      get(const char* section, const char* name, unsigned def) const;
	long          get(const char* section, const char* name, long def) const;
	unsigned long get(const char* section, const char* name, unsigned long def) const;
	float         get(const char* section, const char* name, float def) const;
	double        get(const char* section, const char* name, double def) const;
	//
	// Find globally (or in the unnamed section, if GLOBAL_SEARCH_BY_DEFAULT is false)...
	//
	template <typename T> T get(const char* name, T def) const { return get(0, name, def); }


protected:
	//
	// Helpers...
	//
	bool    __check_cfg();

	// ...for init:
	char*  __load(const char* fnam, const char* section = 0);
	int    __preprocess_line(const char* line, char* out, int out_max_size);

	// ...for queries:
	const char*   __findvalue(const char* section, const char* name,
	                          int recursion_limit = -1, const char* lastname = 0) const; // -1 means use Config::SUBST_CHAIN_LIMIT
	const char*   __findsection(const char* name) const;
	const char*   __findkey(const char* name, const char* start = 0, bool section_local = false) const;

	const char* __get_raw(const char* section, const char* name, const char* def) const;
	const char* __get_raw(const char* name, const char* def) const { return __get_raw(0, name, def); }

	//
	// Debug...
	//
//#ifndef NDEBUG
	public: void dump() const;
//#endif

	//
	// Data...
	//
  public:
	const Config cfg = Config();

	unsigned int       explicit_input_encoding_detected_{};  // 0 if none detected (recognized).
	unsigned int       input_encoding_used_{}; // Either the recognized explicit enc., or UTF-8 (as fallback).
	char*              buffer_{};
	// Stats/diagnostics...
	unsigned long      buffer_size_{};
	unsigned           lines_seen_{};    // Lines encountered while reading the input.
	unsigned           lines_kept_{};    // Lines kept in memory after preprocessing.
	unsigned           syntax_errors_{};

}; // IniMan


//--------------------------------------------------------------------
// Specialized getter templates...
//--------------------------------------------------------------------
template<>
const char* IniMan::get(const char* name, const char* def) const { return __get_raw(0, name, def); }


} // namespace

#endif // _INI_D2FIEU5GDUY43F5NG8YRGYU356F_
