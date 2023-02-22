#ifndef __STRINGTOOLS_HPP__
#define __STRINGTOOLS_HPP__

#include <string>
#include <cstring>

namespace misc {

bool escape_quotes(std::string& str, char quote = '"', char escmark = '\\')
// Intended to supplement istream >> std::quoted(...).
// And that also requires escaping the escape char, too.
{
	bool changed = false;
	for (size_t pos = 0; pos < str.size(); ++pos) { //! str may grow, so < .size() is mandatory!
		if (str[pos] == quote || str[pos] == escmark) {
			str.insert(pos, 1, escmark);
			++pos;
			changed = true;
		}
	}
	return changed;
}

bool escape_chars(std::string& str, const char* escapees, char escmark = '\\')
{
	bool changed = false;
	for (size_t pos = 0; pos < str.size(); ++pos) { //! str may grow, so < .size() is mandatory!
		if (std::strchr(escapees, str[pos] || str[pos] == escmark)) {
			str.insert(pos, 1, escmark);
			++pos;
			changed = true;
		}
	}
	return changed;
}

}; // namespace

#endif // __STRINGTOOLS_HPP__