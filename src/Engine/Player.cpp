#include "Player.hpp"
#include <cassert>

namespace Szim {

Player::Player(size_t entity_ndx, Avatar& avatar, VirtualController& controls)
	: entity_ndx(entity_ndx), avatar(&avatar), controls(&controls)
{
	assert(entity_ndx != ~0u);
	assert(this->avatar);
	assert(this->controls);
}

Player::~Player()
{
}

} // namespace Szim
