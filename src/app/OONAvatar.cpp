#include "OONAvatar.hpp"
#include <sz/sys/fs.hh>
#include <string_view>
	using std::string_view;

#include "Szim/diag/Error.hpp"
//#include "Szim/diag/Log.hpp"

namespace OON {

/*static*/ const char* OONAvatar::prefix_path = "";


OONAvatar::OONAvatar(const Szim::Avatar& a, _LoadFlags load_mode, const char* prefix_path_arg)
	: Avatar{.image_path = a.image_path, .tint_RGBA = a.tint_RGBA}
{
	if (load_mode == PRELOAD) {
		if (!load(prefix_path_arg)) {
			//!!?? Disable the instance, or something?...
		}
	}
}

bool OONAvatar::load(const char* prefix_path_arg) /*!!override!!*/
{
	auto prefix = prefix_path_arg && *prefix_path_arg
	            ? prefix_path_arg : prefix_path;

	auto path = sz::fs::prefix_by_intent(prefix, image_path);
	if (!image.load(path)) {
		Error("Failed to load image \"" + path + "\"!");
		return false;
	}
	return true;
}

} // namespace OON
