#include "OONAvatar_sfml.hpp"
#include <sz/sys/fs.hh>
#include <string_view>
	using std::string_view;

#include "Engine/diag/Error.hpp"
//#include "Engine/diag/Log.hpp"

namespace OON {

/*static*/ const char* Avatar_sfml::prefix_path = "";


Avatar_sfml::Avatar_sfml(const Szim::Avatar& a, _LoadFlags load_mode, const char* prefix_path_arg)
	: Avatar{.image_path = a.image_path, .tint_RGBA = a.tint_RGBA}
{
	if (load_mode == PRELOAD) {
		if (!load(prefix_path_arg)) {
			//!!?? Disable the instance, or something?...
		}
	}
}

bool Avatar_sfml::load(const char* prefix_path_arg) /*!!override!!*/
{
	auto prefix = prefix_path_arg && *prefix_path_arg
	            ? prefix_path_arg : prefix_path;

	auto path = sz::prefix_if_rel(prefix, image_path);
	if (!image.loadFromFile(path)) {
		Error("Failed to load image \"" + path + "\"!");
		return false;
	}
	return true;
}

} // namespace OON
