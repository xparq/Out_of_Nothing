#ifndef _SZ_FS_HH_
#define _SZ_FS_HH_

#include <filesystem>
#include <string>

namespace sz {

inline string getcwd()
{
	std::error_code ec;
	std::filesystem::path cwd = std::filesystem::current_path(ec);
	return ec ? "" : cwd.string();
}

inline string dirname(std::string path)
{
	return std::filesystem::path(path).parent_path().string();
}

}; // namespace
#endif // _SZ_FS_HH_
