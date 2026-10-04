#include "SFML.hpp"

#include <SFML/Window/Mouse.hpp>

namespace sys::adapter::UI::Input
{

bool mouse_button_down(int which)
{
	switch (static_cast<MouseButton>(which)) {
	case Primary:   return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	case Secondary: return sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
	case Middle: // or Wheel:
	                return sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
	case Extra1:    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Extra1);
	case Extra2:    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Extra2);
	case Extra3:
	case Extra4:
	default:        [[fallthrough]];
	}
	return false;
}

} // namespace sys::adapter::UI::Input
