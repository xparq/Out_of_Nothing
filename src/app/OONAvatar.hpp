#ifndef OAV028793NX27V98760C845B7638C45Y6I8BYTHCN8O56
#define OAV028793NX27V98760C845B7638C45Y6I8BYTHCN8O56

#include "Szim/Core/Session/Player/Avatar.hpp" //!!... :-/
#include "SAL/gfx/element/Texture.hpp"

namespace OON {

struct OONAvatar : Szim::Avatar
{
	static const char* prefix_path /*= ""*/;

	SAL::gfx::Texture image;

	enum _LoadFlags { PRELOAD, DEFER_LOADING };
	OONAvatar(const Szim::Avatar& a, _LoadFlags load_mode = PRELOAD, const char* prefix_path = "");
	bool load(const char* prefix_path = "") /*!!override!!*/;
};

} // namespace OON

#endif // OAV028793NX27V98760C845B7638C45Y6I8BYTHCN8O56
