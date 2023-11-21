/*
	v0.0.8
*/

#ifndef _LSF39847G45796GK890G676G42GF35_
#define _LSF39847G45796GK890G676G42GF35_

#include <filesystem>
#include <string>
#include <string_view>
#include <cassert>

namespace sz {

inline std::string getcwd()
{
	std::error_code ec;
	std::filesystem::path cwd = std::filesystem::current_path(ec);
	return ec ? "" : cwd.string();
}

inline std::string dirname(std::string_view path)
{
	return std::filesystem::path(path).parent_path().string();
}

inline std::string basename(std::string_view path, bool keep_last_suffix = true)
{
	return keep_last_suffix ? 
		std::filesystem::path(path).filename().string() :
		std::filesystem::path(path).stem().string();
}

// This one updates the input in-place
inline std::string& endslash_fixup(std::string* dirpath)
{
	assert(dirpath);
	std::string& result = *dirpath;
	if (!result.empty() && result.back() != '/' && result.back() != '\\')
	       result += '/';
	return result;
}

inline std::string endslash_fixup(const std::string& dirpath)
{
	std::string result(dirpath.data(), dirpath.size());
	return endslash_fixup(&result);
}



// Path prefixing with heuristics to only apply the prefix to paths
// that were "probably intended" for it...
template <class Str, typename StrOrCharPtr> // 'path' can be string or string_view, prefix can also be const char*
std::string prefix_if_rel(const StrOrCharPtr& prefix, Str path,
                          bool cwd_not_prefixed = true, bool ignore_backslash = false)
{
#ifdef _WIN32
#  define OR_BACKSLASH_TOO_ON_WINDOWS(ndx) || path[ndx] == '\\' && !ignore_backslash
#  define _P_SEP_ '\\'
#else
#  define OR_BACKSLASH_TOO_ON_WINDOWS(ndx)
#  define _P_SEP_ '/'
#endif
	return path.length() > 0 && (path[0] == '/' OR_BACKSLASH_TOO_ON_WINDOWS(0)) ||
               path.length() > 1 && (path[1] == ':') ||

	     (cwd_not_prefixed && ( // Special-casing ., ./whatever, .\whatever, but not .anything_else
               path.length() == 1 && (path[0] == '.') ||
               path.length() > 1  && (path[0] == '.') && (path[1] == '/' OR_BACKSLASH_TOO_ON_WINDOWS(1))
             ))

		? path
		: (std::filesystem::path(prefix) += path).string();
#ifdef _WIN32
#  undef OR_BACKSLASH_TOO_ON_WINDOWS
#  undef _W_SEP_
#endif
}


}; // namespace sz


//============================================================================
#ifdef UNIT_TEST

#include <string_view>
#include <iostream>

using namespace sz;
using namespace std;

int main()
{
	cerr << "Should be crap/target: " << prefix_if_rel("crap/", "target"s) << '\n';
	cerr << "Should be /target: " <<     prefix_if_rel("crap", "/target"s) << '\n';
	cerr << "Should be \\target: " <<    prefix_if_rel("crap", "\\target"s) << '\n';
	cerr << "Should be crap\\target: " <<prefix_if_rel("crap", "\\target"s, false, true) << '\n';
	cerr << "Should be \\: " <<          prefix_if_rel("crap", "\\"s) << '\n';
	cerr << "Should be crap\\: " <<      prefix_if_rel("crap", "\\"s, false, true) << '\n';
	cerr << "Should be /: " <<           prefix_if_rel("crap", "/"s) << '\n';
	cerr << "Should be crap: " <<        prefix_if_rel("crap", ""s) << '\n';
	cerr << "Should be c: " <<           prefix_if_rel("crap", "c:"s) << '\n';

	// Special-casing ., ./whatever, .\whatever, but not .anything_else...

	auto test_dot = [](bool enable) {
		cerr << enable << " .: " <<     prefix_if_rel("crap", "."s, enable) << '\n';
		cerr << enable << " ./: " <<    prefix_if_rel("crap", "./"s, enable) << '\n';
		cerr << enable << " .\\: " <<   prefix_if_rel("crap", ".\\"s, enable) << '\n';
		cerr << enable << " ./keep: " <<prefix_if_rel("crap", "./keep"s, enable) << '\n';
		cerr << enable << " .x: " <<    prefix_if_rel("crap", ".x"s, enable) << '\n';
		cerr << enable << " ..: " <<    prefix_if_rel("crap", ".."s, enable) << '\n';
		cerr << enable << " ../: " <<   prefix_if_rel("crap", "../"s, enable) << '\n';
		cerr << enable << " ..\\: " <<  prefix_if_rel("crap", "..\\"s, enable) << '\n';
		cerr << enable << " ..x: " <<   prefix_if_rel("crap", "..x"s, enable) << '\n';
	};

	// ENABLED:
	test_dot(true);

	// DISABLED:
	test_dot(false);
}
#endif // UNIT_TEST
#endif // _LSF39847G45796GK890G676G42GF35_
