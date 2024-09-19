#include "iniman.hh"

#include <cstdlib>
#include <cstring> // memmove, ...
//!! Use this instead of atoi etc.: #include <charconv>
#include <cstdio>
#include <cerrno>
#include <cassert>


//===========================================================================
// Staged for sz.lib...
//===========================================================================

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cctype> // isspace, toupper, ...

namespace INIMAN_NAMESPACE //!! Moved up here to enclose sz::, to avoid collisions with real sz:: stuff!
{
namespace sz {
	using byte = std::uint8_t; // std::byte is idiotically restricted
	using uint = unsigned;

	using u8  = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;
	using i8  = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using namespace std; // <- Keep this only in the .cc when moved to the lib...

//---------------------------------------------------------------------------
//  Strip leading/ending whitespace (or other) chars, in-place
//
//  Returns `src`.
//
char* strtrim(char* src, const char* leading_junk = " \t\v\r\n\f")
{
	const char*         res;
	const char*         ptr;
	size_t              newend;

	assert(src);

	// Skip leading junk...
	res = src + strspn(src, leading_junk);
	// Find the *last* non-space char...
	for (newend = 0, ptr = res; *ptr; ++ptr)
	{
		if (!isspace(*ptr))
			newend = ptr - res + 1;
	}
	// Move the string left...
	memmove(src, res, newend + 1);
	src[newend] = '\0';

	return src;
}

//---------------------------------------------------------------------------
char* strupr(char* s)
{
	for (char* r = s; *r; ++r) *r = (char)toupper(*r);
	return s;
}

namespace UTF
{
	enum Encoding : uint32_t { //! The values kinda match the BOM they are inferred from,
	                           //! but 32/BE had to be contorted so the enum values can be unique... :-/
		Unknown,
		UTF8     = 0xEFBBBF,
		UTF16_LE = 0xFFFE,     UTF16_BE = 0xFEFF,
		UTF32_LE = 0xFFFE0000, UTF32_BE = 0xFEFF0000/* HI/LO halves swapped! */,
	};

	struct BOM { byte bytes[4]; u8 len; };

	Encoding detect_encoding(FILE* f)
	// Checks the presence (& value) of a BOM in the opened file f, at the
	// current position.
	//
	// Moves the file position past the BOM, if one was recognized, else
	// restores it to where it was.
	//
	// Any IO errors are deferred to the caller.
	{
		long fpos = ftell(f);

		auto freset  = [f, fpos] (uint offset = 0) { fseek(f, fpos + offset, SEEK_SET); };
		auto getbyte = [f] () { int c = fgetc(f);
//!!			if (c == EOF) ... What now, deep in here?...
			return c;
		};

		Encoding result = Encoding::Unknown;
		freset(); if (getbyte() == 0xEF && getbyte() == 0xBB && getbyte() == 0xBF)
				{ result = Encoding::UTF8; goto done; }
		freset(); if (getbyte() == 0xFF && getbyte() == 0xFE)
				{ result = Encoding::UTF16_LE; goto done; }
		freset(); if (getbyte() == 0xFE && getbyte() == 0xFF)
				{ result = Encoding::UTF16_BE; goto done; }
		freset(); if (getbyte() == 0xFF && getbyte() == 0xFE && getbyte() == 0x00 && getbyte() == 0x00)
				{ result = Encoding::UTF32_LE; goto done; }
		freset(); if (getbyte() == 0x00 && getbyte() == 0x00 && getbyte() == 0xFE && getbyte() == 0xFF)
				{ result = Encoding::UTF32_BE; goto done; }
	done:
		if (result == Encoding::Unknown)
			freset(); // Restore the file pos., if not detected!

		return result;
	}

	const char* encoding_name(Encoding encoding)
	{
		switch (encoding) {
			case Encoding::UTF8:     return "UTF-8";
			case Encoding::UTF16_LE: return "UTF-16/LE";
			case Encoding::UTF16_BE: return "UTF-16/BE";
			case Encoding::UTF32_LE: return "UTF-32/LE";
			case Encoding::UTF32_BE: return "UTF-32/BE";
			default: return "unknown encoding";
		}
	}

	BOM encoding_BOM(Encoding encoding)
	{
		switch (encoding) {
			case Encoding::UTF8:     return { {0xEF, 0xBB, 0xBF}, 3 };
			case Encoding::UTF16_LE: return { {0xFF, 0xFE}, 2 };
			case Encoding::UTF16_BE: return { {0xFE, 0xFF}, 2 };
			case Encoding::UTF32_LE: return { {0xFF, 0xFE, 0x00, 0x00}, 4 };
			case Encoding::UTF32_BE: return { {0x00, 0x00, 0xFE, 0xFF}, 4 };
			default:                 return { {0,0,0,0}, 0 };
		}
	}
} // namespace UTF

//---------------------------------------------------------------------------
// Returns # of chars of the line read, or EOF after the last line.
// NOTE: All but the last lines end with 'LF' and the last ends with EOF.
//---------------------------------------------------------------------------
int loadline(FILE* f, char* line, int linemaxlen)
{
	int     cnt;        // # of chars loaded into the buffer
	int     c;          // the last char read

	assert(linemaxlen > 0);

	// Read until LF (or CR!) or EOF...
	for (cnt = 0; c = fgetc(f), c != '\n' && c != '\r' && c != EOF;)
	{
		// put char into the buffer...
		if (cnt + 1 < linemaxlen)
			line[cnt++] = (char)c;
	}

	// Append EOS...
	line[cnt] = '\0';

	return (cnt == 0 && c == EOF) ? EOF : cnt;
}

//---------------------------------------------------------------------------
struct File
{
	File(const char* name, const char* flags = "rb") { f = fopen(name, flags); }
	~File() { if (autoclose && f) fclose(f); }
	File(const File&) = delete;
	//!!...File(File&& src) { ... } //!! Can't have std::swap: <algorithm> is a ridiculous burden nowadays! :-/

	operator bool () const { return f && !ferror(f) && !eof(); }
	bool eof() const { return f ? feof(f) : false; }

	unsigned long size() //const
	{
		long size = 0; //!! Long is too small for big files! (Hopefully not an issue for INI files though... ;) )
		if (f) //!! This is a landmine, though, with sloppy error checking at the client!
		{
			//!! Still a landmine...: if f was opened in text mode, it's actually UNSPECIFIED! And also WRONG...
			fseek(f, 0, SEEK_END);
			size = ftell(f) + 2; // + EOS + my paranoia... (but also: it says it'll have a double EOS at the end!)
			fseek(f, 0, SEEK_SET);
		}
		return size;
	}

	FILE* handle() const { return f; }
protected:
	bool autoclose = true;
	FILE* f = 0;

	// Reuse existing handle (intended for stdin/stdout/stderr, mostly!):
	File(FILE* h) : autoclose(false), f(h) {}
};

} // namespace sz


//===========================================================================
//===========================================================================
//===========================================================================


//!! Moved up to enclose sz::, to avoid collisions with other places also having sz::File etc...:
//!!namespace INIMAN_NAMESPACE
//!!{

static inline const char EOS = '\0';
static inline const char BC_SUBST = '\x1A';


#define INI_ERROUT(...) (Config::CONSOLE_MESSAGES ? fprintf(stderr, __VA_ARGS__) : 0)
#define INI_ERR(...) (INI_ERROUT("- [IniMan] "),   INI_ERROUT(__VA_ARGS__), INI_ERROUT("\n"))
#define INI_DBG(...) (INI_ERROUT("[IniMan DBG> "), INI_ERROUT(__VA_ARGS__), INI_ERROUT("]\n"))


//---------------------------------------------------------------------------
// CONSTR/DESTR...
//---------------------------------------------------------------------------

//!! IniMan::Config::Config()
bool IniMan::Config::valid() const
{
	bool _valid = false;
	// Some arbitrary checks...:
	if (MAX_LINE_SIZE < 3)
		INI_ERR("ERROR: Line size limit too low (%u)!", MAX_LINE_SIZE);
	else if (SUBST_CHAIN_LIMIT < 0 || SUBST_CHAIN_LIMIT > 1000)
		INI_ERR("ERROR: Invalid ref. chain length limit (%u)!", SUBST_CHAIN_LIMIT);
	else
		_valid = true;

	return _valid;
}


//---------------------------------------------------------------------------
IniMan::IniMan(const Config& cfg)
:	cfg(cfg)
{
	__check_cfg();

	buffer_ = strdup("");  // Start with an empty list //!! BUT!... -> #40, #38!
}

//---------------------------------------------------------------------------
IniMan::IniMan(const char* fnam, const char* section)
:	IniMan(Config(), fnam, section) {}

IniMan::IniMan(const Config& cfg, const char* fnam, const char* section)
:	cfg(cfg)
{
	if (!__check_cfg()) {
		buffer_ = strdup("");  // Start with an empty list //!! BUT!... -> #40, #38!
		return;
	}

	__load(fnam, section);
	if (!buffer_) {
		INI_ERR("ERROR: Failed to load '%s'!", fnam);
	}
}


//---------------------------------------------------------------------------
IniMan::~IniMan()
{
	if (buffer_) free(buffer_);
}


//---------------------------------------------------------------------------
void IniMan::clear()
{
	if (buffer_) free(buffer_);
	buffer_ = strdup(""); //!! BUT... -> #40, #38!

	explicit_input_encoding_detected_ = 0;
	input_encoding_used_ = 0;
	buffer_size_ = 0;
	lines_seen_ = 0;
	lines_kept_ = 0;
	syntax_errors_ = 0;
}


//---------------------------------------------------------------------------
// QUERIES...
//---------------------------------------------------------------------------

namespace impl {
using Config = IniMan::Config;
int _base(bool cfg_oct_enabled, const char** input)
{
	assert(input);
	assert(*input);
	assert(**input);
	const char* s = *input;
	int b = 0; // "detect" for strto*

	// Disable octal!
	if (!cfg_oct_enabled
		&&  s[0] == '0'
		&& (s[1] != 'X' && s[1] != 'x')
		&& (s[1] != 'B' && s[1] != 'b')) b = 10;

	// strto* doesn't seem to detect binary, so...:
	if (s[0] == '0' && (s[1] == 'B' || s[1] == 'b'))
		{ b = 2; *input += 2; }

	return b;
}

#define _STRIFY_(x) #x

void _report_format_error(const char* input, const char* type)
	{ INI_ERR("ERROR: failed to get '%s' as '%s'", input, type); }
void _report_overflow(const char* input, const char* type)
	{ INI_ERR("Warning: '%s' is too big for '%s'", input, type); }
} // namespace impl

#define _NUM_CONV_IMPL_(TargetType, strtoT, Section, Key, Defval) \
	const char* ptr = __findvalue(Section, Key); \
	if (!ptr || !*ptr) return Defval; \
	int b = impl::_base(cfg.NUM_C_OCTAL, &ptr); \
	char* end; auto val = strtoT(ptr, &end, b); \
	if (ptr == end) { \
		impl::_report_format_error(ptr, _STRIFY_(TargetType)); \
		val = Defval; \
	} else if (errno == ERANGE) { \
		errno = 0; \
		impl::_report_overflow(ptr, _STRIFY_(TargetType)); \
        } \
	return static_cast<TargetType>(val);

#define _NUM_CONV_F2_(F) [](auto p, auto e, [[maybe_unused]] auto dummy) { return F(p, e); }


//---------------------------------------------------------------------------
const char* IniMan::__get_raw(const char* section, const char* name, const char* def) const
{
	const char* ptr = __findvalue(section, name);
	return ptr ? ptr : (def ? def : "");
}


//---------------------------------------------------------------------------
int IniMan::get(const char* section, const char* name, int def) const
{
	_NUM_CONV_IMPL_(int, strtol, section, name, def)
}

//---------------------------------------------------------------------------
unsigned IniMan::get(const char* section, const char* name, unsigned def) const
{
	_NUM_CONV_IMPL_(unsigned, strtoul, section, name, def)
}

//---------------------------------------------------------------------------
long IniMan::get(const char* section, const char* name, long def) const
{
	_NUM_CONV_IMPL_(long, strtol, section, name, def)
}

//---------------------------------------------------------------------------
unsigned long IniMan::get(const char* section, const char* name, unsigned long def) const
{
	_NUM_CONV_IMPL_(unsigned long, strtoul, section, name, def)
//	const char* ptr = __findvalue(section, name);
//	return ptr ? atol(ptr) : def;
}

//---------------------------------------------------------------------------
float IniMan::get(const char* section, const char* name, float def) const
{
	_NUM_CONV_IMPL_(float, _NUM_CONV_F2_(strtof), section, name, def)
//	const char* ptr = __findvalue(section, name);
//	return ptr ? atof(ptr) : def;
}

//---------------------------------------------------------------------------
double IniMan::get(const char* section, const char* name, double def) const
{
	_NUM_CONV_IMPL_(float, _NUM_CONV_F2_(strtod), section, name, def)
}

#undef _NUM_CONV_IMPL_
#undef _STRIFY_

//---------------------------------------------------------------------------
// INTERNAL HELPERS...
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool IniMan::__check_cfg()
{
	if (cfg.valid()) return true;

	INI_ERR("ERROR: Invalid config; this instance is unusable.");
	return false;
}


//---------------------------------------------------------------------------
// NOTE: not case sensitive
const char* IniMan::__findsection(const char* section) const
{
	const char* ptr = buffer_;
	size_t      s;

	assert(section);
	if (!valid()) return 0; //!!?? Or just assert?... -> #18

	// prepare section-name string...
	char linebuf[Config::MAX_LINE_SIZE]; memset(linebuf, 0, Config::MAX_LINE_SIZE);
	linebuf[0] = '[';
	strncpy(linebuf + 1, section, Config::MAX_LINE_SIZE - 3); // '[' + ']' + EOS
	linebuf[Config::MAX_LINE_SIZE - 2] = '\0';
	strcat(linebuf, "]");
	sz::strupr(linebuf);

	while ((s = strlen(ptr)) != 0)
	{
		if (strcmp(ptr, linebuf) == 0)
			return ptr + s + 1;
		ptr = ptr + s + 1;
	}

	return 0;
}

//---------------------------------------------------------------------------
// NOTE: not case sensitive
const char* IniMan::__findkey(const char* name, const char* start, bool section_local) const
{
	const char* ptr;
	size_t      s;

	assert(name);
	if (!valid()) return 0; //!!?? Or just assert?...

	// search from...
	ptr = (start ? start : buffer_);

	// prepare key-name string...
	char linebuf[Config::MAX_LINE_SIZE]; memset(linebuf, 0, Config::MAX_LINE_SIZE);
	strncpy(linebuf, name, Config::MAX_LINE_SIZE - 2); // '=' + EOS
	strcat(linebuf, "=");
	sz::strupr(linebuf);
//cerr << "SEARCHING for key '"<<name<<"', as in the linebuf: [" << linebuf << "]\n";

	while ((s = strlen(ptr)) != 0)
	{
		if (ptr[0] == '[') // another section reached?
			if (section_local) return 0;

		if (strstr(ptr, linebuf) == ptr)
			return strchr(ptr, '=') + 1;

		ptr = ptr + s + 1;
	}

	return 0;
}

//---------------------------------------------------------------------------
const char* IniMan::__findvalue(const char* section, const char* key,
	// For subst. processing:
	int reclimit/* = -1*/, [[maybe_unused]] const char* lastkey/* = 0*/) const
{
#define _SUBST_MODE_ (reclimit != cfg.SUBST_CHAIN_LIMIT)

	if (reclimit == -1) reclimit = cfg.SUBST_CHAIN_LIMIT; // Can't just have it as a default param directly...

	const char* ptr = 0;

	if (section && *section)
	{
		if ((ptr = __findsection(section)) == 0)
		{
//INI_DBG("section not found: '%s'", section);
			return 0;   // section not found
		}
	}

	const char* val = __findkey(key, ptr, section || !cfg.GLOBAL_SEARCH_BY_DEFAULT);
	if (!val)
	{
//INI_DBG("key not found: '%s'", key);
		if (_SUBST_MODE_) // Are we resolving a reference?
		{
			if (cfg.SUBST_ENV)
			{
//INI_DBG("looking up %s as env. var...", key);
				if (const char* ev = getenv(key))
					return ev;
			}

			//!! Cringy attempt to backtrack from unresolved reference to literal value...
			if (cfg.KEEP_UNRESOLVED_REF_AS_IS)     // ...and reusing the ref. name is enabled!
			{
				//!! WOAH!... :)) Have you seen anything like this before?! ;)
				assert(key > buffer_); // This is not assurance, only encouragement! ;)
				if (*(key - 1) == BC_SUBST) //!! But also %...% or ${...}, one day!...
					return key - 1;
			}
		}

		return 0; // E.g. no buffer, even...
	}

	assert(buffer_);

	if (!cfg.SUBSTITUTION)
		return val;

	//
	// Process "variable" substitution...
	//

	if (*val != BC_SUBST) //!! %...% or ${...} can't be supported yet!
		return val;

//INI_DBG("resolving ref.: '%s'", val);
	++val;
	if (reclimit == 0)
	{
		INI_ERR("ERROR: Substitution depth limit (%u) reached at resolving '%s'", cfg.SUBST_CHAIN_LIMIT, val);
		return 0;
	}
	return __findvalue(section, val, reclimit - 1, key);

#undef _SUBST_MODE_
}



//---------------------------------------------------------------------------
// Returns 0 if OK, else an error code. (See "enum {...} err" below.)
// NOTE: The processed line is never longer than the raw string.
//---------------------------------------------------------------------------

enum ParseResult : int { ABORT = -1, OK = 0, EMPTY, SYNTAX_UNEXPECTED, SYNTAX_INCOMPLETE };
	// ABORT is only used as a return value of __preprocess_line, not for its own processing!

int IniMan::__preprocess_line(const char* line, char* output, int outbufsize)
{
#define __PUTCH(c)  if (wr - output + 1 < outbufsize) (*wr++ = (char)c)
#define __TOUPPER(c)  (c = (char)toupper(c))
#define __IS_COMMENT(c)  (c == ';' || c == '#')
//#define __IS_COMMENT(c)  (c == '/' && *rd == '/')

	enum { SEEK, SECTION, KEY, PREVALUE, VALUE, POSTQUOTE, EOL } mode;
	enum { WRITE, SKIP, RESCAN, STOP } cmd;
	ParseResult result;

	bool quoted = false; // quoting is useful for " strings with *surrounding* spaces "...

	int c = 0;
	const char* rd;
	char* wr;

	rd = line;      // read head
	wr = output;    // write head
	mode = SEEK;
	result = EMPTY;
	cmd = WRITE;

	do
	{
		if (cmd != RESCAN)
			c = *rd++;

		cmd = WRITE;

		switch (mode)
		{
		case SEEK:
			if (isspace(c))
			{
				cmd = SKIP;   // skip leading whitespace
			}
			else if (c == '[')
			{
				result = OK;
				mode = SECTION;
			}
			else if (c == EOS || __IS_COMMENT(c))
			{
				result = EMPTY;
				cmd = STOP;
			}
			else
			{
				result = OK;
				mode = KEY;
			}
			break;

		case SECTION:
			if (c == ']')
			{
				mode = EOL;
			}
			else if (c == EOS || !quoted && __IS_COMMENT(c))
			{
				result = SYNTAX_INCOMPLETE;
				cmd = STOP;
			}
			else if (!quoted && c == '\"')   // open quote...
			{
				quoted = true;
				cmd = SKIP;
			}
			else if (quoted && c == '\"')  // close quote...
			{
				quoted = false;
				cmd = SKIP;
			}
			__TOUPPER(c);
			break;

		case KEY:
			if (c == '=')   // a value is coming?
			{
				// trim spaces around the name...
				*wr = EOS;  //! "Tokenize" the key.
				sz::strtrim(output);
				wr = output + strlen(output);
				mode = PREVALUE;
			}
			else if (c == EOS || __IS_COMMENT(c))
			{
				result = SYNTAX_INCOMPLETE;
				cmd = STOP;
			}
			break;

		case PREVALUE:
			if (isspace(c))
			{
				cmd = SKIP;   // skip spaces after the '='
			}
			else if (cfg.SUBSTITUTION && c == '$')
			{
				// Replace it with the "bytecode" of ASCII 'SUBSTITUTE' (0x1A, ^Z) :)
				*wr++ = BC_SUBST;
				//!! Extend
				//!! - to allow $ anywhere else, not just as a value prefix
				//!! - to optional ${key} refs, with another BC_... prefix + a key size
				//!!   byte replacing the '{', and then eating up its '}' pair... Plus
				//!!   + extra logic in __findvalue to continue interpreting the value for
				//!!   other possible references (perhaps assisted with a line prefix byte code
				//!!   flag (allowed by the eaten-up closing '}' telling whether there are any!...)

				mode = VALUE;
				cmd = SKIP;
			}
			else
			{
				mode = VALUE;
				cmd = RESCAN;
			}
			break;

		case VALUE:
			if (!quoted && __IS_COMMENT(c))  // comment?
			{
				mode = EOL;
				cmd = SKIP;
			}
			else if (!quoted && c == '\"')   // open quote...
			{
				quoted = true;
				cmd = SKIP;
			}
			else if (quoted && c == '\"')  // close quote...
			{
				quoted = false;
// Disabled as of #49:		mode = POSTQUOTE;
				cmd = SKIP;
			}
			else if (c == EOS)
			{
				mode = EOL;
				cmd = RESCAN;   // this prevents auto-exiting on EOS
			}
			break;
		case POSTQUOTE:
/* Disabled as of #49:
			if (c == EOS)           // end with no error -- Why not with OK??!!
			{
				cmd = STOP;
			}
			else if (isspace(c))
			{
				cmd = SKIP;
			}
			else if (__IS_COMMENT(c))
			{
				cmd = STOP;     // end with no error -- Why not with OK??!!
			}
			else                    // end with error (could as well just warn and ignore the extra junk, though...)
			{
				result = SYNTAX_UNEXPECTED;
				cmd = STOP;
			}
*/
			break;
		case EOL:
			// trim trailing spaces...
			*wr = EOS;  //! The output has not been EOS-terminated yet!
			sz::strtrim(output);
			wr = output + strlen(output);
			cmd = STOP;
			break;
		}

		// process the char...
		switch (cmd)
		{
		case RESCAN:
			break;

		case WRITE:
			if (result == OK)
			{
				if (mode == KEY || mode == SECTION)
				{
					__TOUPPER(c);
				}

				__PUTCH(c);   // (this will write the EOS, too, in turn)
			}

			[[fallthrough]];

		default:
			if (c == EOS)
			cmd = STOP;
			break;
		}

	} while (cmd != STOP);

	// force EOS...
	*wr = EOS;

	// Error details...
	if (result != OK && result != EMPTY)
	{
		++syntax_errors_;

		unsigned    pos = unsigned(rd - line);
		const char* errmsg;
		switch (result)
		{
		case SYNTAX_INCOMPLETE: errmsg = "incomplete term"; break;
		case SYNTAX_UNEXPECTED: errmsg = "unexpected input"; break;
		default: errmsg = "";
		}
		INI_ERR("SYNTAX ERROR (line %u, pos %u): %s\n\t%s", lines_seen_, pos, errmsg, line);
		if (pos < 79) // Try to dodge the mess with overflowing lines...
		{
			INI_ERROUT("\t");
			for (int count = pos; --count;) INI_ERROUT(" ");
			INI_ERROUT("^\n");
		}

		if (!cfg.CONTINUE_ON_SYNTAX_ERRORS) result = ABORT;
	}

	return result;

#undef __PUTCH
#undef __TOUPPER
#undef __IS_COMMENT
}


//---------------------------------------------------------------------------
//
char* IniMan::__load(const char* fnam, const char* section)
{
	if (!__check_cfg()) return 0; // #40

	char linebuf[Config::MAX_LINE_SIZE]; memset(linebuf, 0, Config::MAX_LINE_SIZE);
	char procbuf[Config::MAX_LINE_SIZE];

	using namespace sz; using namespace sz::UTF;

	File f(fnam, "rt");
	if (!f) {
		INI_ERR("ERROR: Failed to open '%s'! (errno: %i)", fnam, errno);
		return 0;
	}

	// Get buffer size big enough for the file... (!!BUT: #44)
	buffer_size_ = f.size() + 2; // + EOS + my paranoia... (but also: it says it'll have a double EOS at the end!)

	//
	// Determine the (input) encoding...
	//
	explicit_input_encoding_detected_ = detect_encoding(f.handle());
	switch (explicit_input_encoding_detected_)
	{
	case Encoding::Unknown:
	default:
		INI_ERR("Note: No explicit encoding detected for '%s', assuming UTF-8...", fnam);
		input_encoding_used_ = Encoding::UTF8;
		break;
	case Encoding::UTF16_LE:
	case Encoding::UTF16_BE:
	case Encoding::UTF32_LE:
	case Encoding::UTF32_BE:
		INI_ERR("ERROR: Input encoding %s is not supported for '%s'!", encoding_name(Encoding(explicit_input_encoding_detected_)), fnam);
		assert(input_encoding_used_ == 0);
		assert(Encoding::Unknown == 0);
		return 0;
	case Encoding::UTF8:
		INI_ERR("Note: Expliciti UTF-8 encoding detected for '%s'.", fnam);
		input_encoding_used_ = Encoding::UTF8;
		break;
	}
	assert(Encoding::Unknown == 0);
	assert(input_encoding_used_ != Encoding::Unknown); // explicit_input_encoding_detected_ can be Unknown (0)

	//
	// Allocate space of sufficient size...
	//
	//!!
	//!! -> #38!
	//!!
	buffer_ = (char*)malloc(buffer_size_);
	if (!buffer_)
	{
		INI_ERR("ERROR: Couldn't allocate file buffer for '%s'!", fnam);
		return 0;
	}
	assert(buffer_);

	char* bufferend = buffer_;
	bufferend[0] = EOS;

	//
	// locate the desired section (if one was specified)...
	//
	if (section && *section)
	{
		// prepare the section name...
		char secname_normalized[Config::MAX_LINE_SIZE] = "[";
		strncpy(secname_normalized + 1, section, cfg.MAX_LINE_SIZE - 3); // 3: `[` + `]` + EOS
		secname_normalized[cfg.MAX_LINE_SIZE - 2] = '\0';
		strcat(secname_normalized, "]");
		sz::strupr(secname_normalized);

		// eat up text until the desired section name found...
		while (EOF != sz::loadline(f.handle(), linebuf, cfg.MAX_LINE_SIZE))
		{
			++lines_seen_;

			if (int result = __preprocess_line(linebuf, procbuf, cfg.MAX_LINE_SIZE); result != 0)
			{
				if (result == ABORT) {
					clear();    //!! #40, #38
					return 0;
				} else continue;
			}

			++lines_kept_;

			if (strstr(procbuf, secname_normalized) == procbuf) // found?
			{
/* - uncomment this if you want to...
				// add the name of the single section to the list...
				strcpy(bufferend, procbuf);
				bufferend += strlen(bufferend) + 1;
*/
				goto section_found;
			}
		}

//section_NOT_found:
		INI_ERR("ERROR: Couldn't find section [%s]!", section);
		clear();    //!! #40, #38
		return 0;

//////////
	// *Sigh*, we're at C++23 *just for this*, but MSVC (19.40) still needs a semicolon... ;-/
	// OK, OK, fine, fine... It's C++, I know. You can't win.
section_found:
#ifdef _MSC_VER
	; // (Also preparing for universal semicolon elision in C++32...)
#endif
	// And yes, no, I'm not willing to just write `section_found: ;` as every sane person would. ;-p
	// (I'm doing C++, how could I be a sane person?)
//////////

	} // if (single-section-load)

	//------------------------------
	// Read the entries...
	// (up to the next section only, if single-section-load)...
	//
	while (EOF != sz::loadline(f.handle(), linebuf, cfg.MAX_LINE_SIZE))
	{
		++lines_seen_;

		if (int result = __preprocess_line(linebuf, procbuf, cfg.MAX_LINE_SIZE); result != 0)
		{
			if (result == ABORT) {
				clear();    //!! #40, #38
				return 0;
			} else continue;
		}

		++lines_kept_;

		if (section && *section && procbuf[0] == '[')
		{
			break;  // end of section found
		}
		else
		{
			// add the line to the buffer
			strcpy(bufferend, procbuf);
			bufferend += strlen(bufferend) + 1;
		}
	}

	*bufferend = EOS;   // double EOS at the end!

	return buffer_;
}



//---------------------------------------------------------------------------
void IniMan::dump() const
{
#ifndef NDEBUG
	char*   ptr = buffer_;

	if (!valid()) {
		INI_ERR("WARNING: No content!");
		return;
	}

	assert(buffer_);

	printf(">>>========>>> (buffer [%p] size: %lu)\n", (void*)buffer_, buffer_size_);
	printf(">>> BOM detected:");
		auto [fBOM, fBOMlen] = sz::UTF::encoding_BOM(sz::UTF::Encoding(explicit_input_encoding_detected_));
		for (unsigned i = 0; i < fBOMlen; ++i) { printf(" %02X", fBOM[i]); }
		if (fBOMlen) printf(" (inferred encoding: %s (%08x))",
			sz::UTF::encoding_name(sz::UTF::Encoding(explicit_input_encoding_detected_)),
			explicit_input_encoding_detected_);
		printf("\n");
	printf(">>> BOM for writing:");
		auto [oBOM, oBOMlen] = sz::UTF::encoding_BOM(sz::UTF::Encoding(input_encoding_used_));
		for (unsigned i = 0; i < oBOMlen; ++i) { printf(" %02X", oBOM[i]); } printf("\n");
	printf(">>> 1st char: %x\n", *ptr);
	printf(">>> 1st non-comment line: %s\n", ptr);
	printf(">>>========>>> (lines total: %u, kept: %u; errors: %u)\n", lines_seen_, lines_kept_, syntax_errors_);
	size_t s;
	while ((s = strlen(ptr)) != 0)
	{
		printf("LINE: '%s'\n", ptr);
		ptr = ptr + s + 1;
	}
	printf("<<<========<<<\n");
#endif // NDEBUG
}

} // namespace INIMAN_NAMESPACE



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef TEST

//#include <cstring>
#include <string_view>
#include <string>
//#include <cstddef>

namespace test {
namespace sz {
inline std::string dirname(std::string_view path) {
    std::size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? "" : std::string(path.substr(0, pos));
}

inline std::string basename(std::string_view path, bool keep_last_suffix = true) {
    std::size_t pos = path.find_last_of("/\\");
    std::string_view filename = (pos == std::string::npos) ? path : path.substr(pos + 1);

    if (!keep_last_suffix) {
        std::size_t dot_pos = filename.find_last_of('.');
        if (dot_pos != std::string::npos) {
            return std::string(filename.substr(0, dot_pos));
        }
    }
    return std::string(filename);
}
} // namespace sz

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

} // namespace test

//#define IOSTREAM
#ifdef IOSTREAM
#	include <iostream>
	using std::cout, std::cerr;
#else

namespace INIMAN_NAMESPACE
{
namespace sz {
struct Stream : public File
{
	Stream(FILE* h) : File(h) {}
};

template <typename T> struct _format
{
	static constexpr const char* fmt = "%s";
	static constexpr const char* conv(T) { return "<UNKNOWN TYPE>"; }
};

#define _DEF_STREAM_TRAIT_(Type, Fmt, TargetType, CodeBlock) template <> struct _format<Type> { \
	static constexpr const char* fmt = Fmt; \
	static constexpr TargetType conv(Type x) CodeBlock }

// Extend as needed...:
_DEF_STREAM_TRAIT_(const char*,    "%s",  const char*, { return x; });
_DEF_STREAM_TRAIT_(char,           "%c",  char, { return x; });
_DEF_STREAM_TRAIT_(int,            "%d",  int,  { return x; });
_DEF_STREAM_TRAIT_(long,           "%ld", long, { return x; });
_DEF_STREAM_TRAIT_(unsigned,       "%u",  unsigned, { return x; });
_DEF_STREAM_TRAIT_(unsigned long,  "%lu", unsigned long, { return x; });
_DEF_STREAM_TRAIT_(bool,           "%d",  int, { return x ? 1 : 0; }); //!! OR: "%c",  char, { return x ? 'T' : 'F'; });
_DEF_STREAM_TRAIT_(float,          "%g",  double, { return x; });
_DEF_STREAM_TRAIT_(double,         "%g",  double, { return x; });

#undef _DEF_STREAM_TRAIT_

template <typename T>
Stream& operator << (Stream& out, T x)             { fprintf(out.handle(), _format<T>::fmt, _format<T>::conv(x)); return out; }

Stream& operator << (Stream& out, const string& x) { fprintf(out.handle(), "%s", x.c_str()); return out; }

} // namespace sz

sz::Stream cout{stdout}, cerr{stderr};
#endif // IOSTREAM

} // namespace INIMAN_NAMESPACE

int main(int argc, char** argv)
{
	using namespace INIMAN_NAMESPACE;

	const char* key;
	const char* section;

	const char* fname = "INI";
	if (argc >= 2)
		fname = argv[1];

//	IniMan::Config::CONSOLE_MESSAGES = false;
	IniMan c(IniMan::Config{
//			.SUBSTITUTION = false,
//			.KEEP_UNRESOLVED_REF_AS_IS = true,
//			.NUM_C_OCTAL = true,
			.SUBST_ENV = true,
			.CONTINUE_ON_SYNTAX_ERRORS = true,
			.GLOBAL_SEARCH_BY_DEFAULT  = false,
		}, fname);
//	IniMan c(fname);

	c.dump();

	if (argc >= 3)
	{
		auto raw_key = argv[2];
		auto [section, key] = test::split(raw_key);
		cout << "Section/Key: [" << section << "]/"<< key << " = " << c.get(section.c_str(), key.c_str(), "<NOT FOUND!>") << "\n";
	}

	if (argc < 2)
	{
		key = "no-key"; section = "no-section";
		cout << section << "." << key << " = " << "'" << c.get(key, "<NONE>") << "'\n";
		cout << section << "." << key << " = " << "'" << c.get(section, key, "<NONE>") << "'\n";

		key = "key-ok"; section = "section-ok";
		cout << section << "." << key << " = " << "'" << c.get(key, "<NONE>") << "'\n";
		cout << section << "." << key << " = " << "'" << c.get(section, key, "<NONE>") << "'\n";

		key = "c"; section = "oo";
		cout << section << "." << key << " = " << "'" << c.get(key, "<NONE>") << "'\n";
		cout << section << "." << key << " = " << "'" << c.get(section, key, "<NONE>") << "'\n";

		if (c.load("types.ini"))
		{
			c.dump();
			section = "string";
//			cout <<"["<< section <<"] get " << key << ": " << "'" << c.get(section, "<NONE>") << "'\n";
//			cout <<"["<< section <<"] get " << key << ": " << "'" << c.get(section, key, "<NONE>") << "'\n";
			section = "int";
			key = "i0";     cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, -1) << "\n";
			key = "i1";     cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0) << "\n";
			key = "ihn";    cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0) << "\n";
			key = "iover";  cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0) << "\n";

			key = "hex16";  cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, -1) << "\n";
			key = "hex17";  cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0) << "\n";
			key = "noctal"; cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0) << "\n";
			key = "bin";    cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0) << "\n";

			section = "float";
			key = "f0";     cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, -1.f) << "\n";
			key = "f1";     cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0.f) << "\n";
			key = "fnexpn"; cout <<"["<< section <<"] get " << key << ": " << "" << c.get(section, key, 0.f) << "\n";
		}

	}
  
//	IniMan::Config::CONSOLE_MESSAGES = true;

	c.clear();
	key = "empty-file"; section = "";
	cout << section << "." << key << " = " << "'" << c.get(key, "<NONE>") << "'\n";
	cout << section << "." << key << " = " << "'" << c.get(section, key, "<NONE>") << "'\n";
	cout << section << "." << key << " = " << "'" << c.get(key, 0) << "'\n";
	cout << section << "." << key << " = " << "'" << c.get(section, key, 0) << "'\n";

	if (c.load(fname, "oo"))
	{
		c.dump();
		cout << section << "." << key << " = " << "'" << c.get(key, "<NONE>") << "'\n";
		cout << section << "." << key << " = " << "'" << c.get(section, key, "<NONE>") << "'\n";
		cout << section << "." << key << " = " << "'" << c.get(key, 0) << "'\n";
		cout << section << "." << key << " = " << "'" << c.get(section, key, 0) << "'\n";
	}

	return 0;
}

#endif // TEST
