#include "Player.hpp"
#include <cassert>

namespace Szim {

using namespace Model;

Player::Player(EntityID entity_ndx, Avatar& avatar, VirtualController& controls)
	: entity_ndx(entity_ndx), avatar(&avatar), controls(&controls)
{
	assert(entity_ndx != Core::Entity::NONE); //!! namespacing mess...
	assert(this->avatar);
	assert(this->controls);
}

Player::~Player()
{
}

} // namespace Szim
