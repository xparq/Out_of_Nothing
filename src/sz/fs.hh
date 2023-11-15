#ifndef _LSFK890G676G42GF35_
#define _LSFK890G676G42GF35_

#include <filesystem>
#include <string>

namespace sz {

inline std::string getcwd()
{
	std::error_code ec;
	std::filesystem::path cwd = std::filesystem::current_path(ec);
	return ec ? "" : cwd.string();
}

inline std::string dirname(const std::string& path)
{
	return std::filesystem::path(path).parent_path().string();
}

inline std::string basename(const std::string& path, bool keep_last_suffix = true)
{
	return keep_last_suffix ? 
		std::filesystem::path(path).filename().string() :
		std::filesystem::path(path).stem().string();
}


inline std::string& endslash_fixup(std::string& dirpath)
{
	if (!dirpath.empty() && dirpath.back() != '/' && dirpath.back() != '\\')
	       dirpath += '/';
	return dirpath;
}

inline std::string endslash_fixup(const std::string& dirpath)
{
	std::string result = dirpath;
	if (!result.empty() && result.back() != '/' && result.back() != '\\')
	       result += '/';
	return result;
}

}; // namespace
#endif // _LSFK890G676G42GF35_
