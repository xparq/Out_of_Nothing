#ifndef _028793NX27V98760C845B7638C45Y6I8BYTHCN8O56_
#define _028793NX27V98760C845B7638C45Y6I8BYTHCN8O56_

#include "Engine/Avatar.hpp"
#include <SFML/Graphics/Texture.hpp>

namespace OON {

struct Avatar_sfml : Szim::Avatar
{
	static const char* prefix_path /*= ""*/;

	sf::Texture image;

	enum _LoadFlags { PRELOAD, DEFER_LOADING };
	Avatar_sfml(const Szim::Avatar& a, _LoadFlags load_mode = PRELOAD, const char* prefix_path = "");
	bool load(const char* prefix_path = "") /*!!override!!*/;
};

} // namespace OON

#endif // _028793NX27V98760C845B7638C45Y6I8BYTHCN8O56_
