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
//  - Mostly UTF8-clean (also refuses to load unsupported UTF encodings).
//    However, its current simplistic upper-casing logic can only support
//    the "regular" cases that old C code (with toupper) also could.
//  - Not 8-bit binary-clean though: ASCII control codes are not guaranteed
//    to be fully preserved. (Binary data should be text-encoded (e.g. base64,
//    uuencode etc.), as usual.)
//
//  There's no plan to explicitly support concurrent use for now. (And it *is
//  not* thread safe, when trying to access (even just read) the same file from
//  multiple instances concurrently. Don't do that. Girls don't like it.)
//


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
		bool LOAD_FROM_STRING = false;          // Treat the `filename_or_content` param. as the content itself?

		//!! Config(); // For validation & error reporting.
		//!! Except... Can't have ctors with desig. init!... :-/
		bool valid() const; //!! So, this needs to be called manually from the IniMan ctors... Great. :-/
	};

public:
	IniMan(const Config& cfg);

	// Load an entire file, or just a section of it:
	IniMan(                   const char* filename_or_content, const char* section = 0);
	IniMan(const Config& cfg, const char* filename_or_content, const char* section = 0);

	~IniMan();

	// No copying:
	IniMan(const IniMan&) = delete;

	bool valid() const { return cfg.valid() && buffer_ != 0 /*!!?? && *buffer_ ??!!*/; static_assert(sizeof(int) >= 4, "The int size must be >= 32 bits!"); };
	void clear();

	bool load(const char* filename_or_content, const char* section = 0);

	//-------------------------------------------------------------------
	// Getters...
	//
	// Return `def` if the item couldn't be found or parsed.
	//
	//-------------------------------------------------------------------
	//
	// Find in a named section...
	//
	const char*   get(const char* section, const char* key, const char* def) const { return __get_raw(section, key, def); }
//	template<typename T> T get(const char*, const char*, T) const;
	// Supported types - plus std::string, if enabled via INIMAN_ENABLE_GET_STD_STRING (see in the .cc):
	int           get(const char* section, const char* key, int def) const;
	unsigned      get(const char* section, const char* key, unsigned def) const;
	long          get(const char* section, const char* key, long def) const;
	unsigned long get(const char* section, const char* key, unsigned long def) const;
	float         get(const char* section, const char* key, float def) const;
	double        get(const char* section, const char* key, double def) const;
	// Unknown-type trap to trigger a compilation error:
	template<typename T> T get(const char*, const char*, T) const
		{ static_assert(sizeof(T) == 0, "Unsupported type for IniMan::get()"); }

	//
	// Find globally (or in the unnamed section, if GLOBAL_SEARCH_BY_DEFAULT is false)...
	//
	template <typename T> T get(const char* key, T def) const { return get(0, key, def); }


protected:
	//
	// Helpers...
	//

	// ...for setup/teardown, housekeeping:
	bool   __check_cfg();
	void   __new_buffer(unsigned long bufsize); // bufsize must not be 0.
	void   __new_buffer(const char* content = ""); // content must not be 0; and must be 0-terminated; will be copied.
		// content MUST be 0-terminated!
	void   __release_buffer();
	char*  __load(const char* filename_or_content, const char* section = 0); // Must be called with no buffer allocated yet!
	                                                                         // (i.e. from a ctor, or after __release_buffer)
	int    __preprocess_line(const char* line, char* out, int out_max_size);

	// ...for queries:
	const char* __get_value(const char* section, const char* key,
	                          int recursion_limit = -1, const char* lastname = 0) const; // -1 means use Config::SUBST_CHAIN_LIMIT
	const char* __find_section(const char* section) const;
	const char* __find_value(const char* key, const char* start = 0, bool section_local = false) const;

	const char* __get_raw(const char* section, const char* key, const char* def) const;
	const char* __get_raw(const char* key, const char* def) const { return __get_raw(0, key, def); }

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
	//!! const <- Had to disable JUST TO BE ABLE to flip LOAD_FROM_STRING after construction, to support the
	//!! OON config loader logic that wants to dispatch between file/string input after creating its IniMan backend...
	Config cfg = Config();

	unsigned           explicit_input_encoding_detected_{};  // 0 if none detected (recognized).
	unsigned           input_encoding_used_{}; // Either the recognized explicit enc., or UTF-8 (as fallback).
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
const char* IniMan::get(const char* key, const char* def) const { return __get_raw(0, key, def); }


} // namespace

#endif // _INI_D2FIEU5GDUY43F5NG8YRGYU356F_
