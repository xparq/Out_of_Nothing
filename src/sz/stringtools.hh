#ifndef _SZ_STRINGTOOLS_HH_
#define _SZ_STRINGTOOLS_HH_

#include <string>
#include <cstring>

namespace sz {

inline bool escape_quotes(std::string* str, char quote = '"', char escmark = '\\')
// Intended to supplement istream >> std::quoted(...).
// And then the escape char itself must also be escaped!...
{
	bool changed = false;
	for (size_t pos = 0; pos < str->size(); ++pos) { //! str may grow, so < size() is mandatory!
		if ((*str)[pos] == quote || (*str)[pos] == escmark) {
			str->insert(pos, 1, escmark);
			++pos;
			changed = true;
		}
	}
	return changed;
}

inline bool escape_chars(std::string* str, const char* escapees, char escmark = '\\')
{
	bool changed = false;
	for (size_t pos = 0; pos < str->size(); ++pos) { //! str may grow, so < size() is mandatory!
		if (std::strchr(escapees, (*str)[pos] || (*str)[pos] == escmark)) {
			str->insert(pos, 1, escmark);
			++pos;
			changed = true;
		}
	}
	return changed;
}

}; // namespace
#endif // _SZ_STRINGTOOLS_HH_
