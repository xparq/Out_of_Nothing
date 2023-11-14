#ifndef _SA37H6FGFGH50V6BC2370_
#define _SA37H6FGFGH50V6BC2370_

#include "Engine/HCI.hpp"
#include "Engine/Config.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/System/String.hpp> //!!?? WTF is this not included already?! :-o I've never had to do this!
//#include <SFML/Window/Context.hpp>
//!!#include <SFML/Window/Event.hpp>

#include <string>

namespace Szim {

struct SFML_HCI : HCI
{
	struct Window : HCI::Window {
		sf::RenderWindow _owned_sfml_window; //! <- Not just a ref!
		Window(unsigned width, unsigned height, const std::string& title)
		{
			_owned_sfml_window.create(sf::VideoMode({width, height}), title);
		}
		operator sf::RenderWindow&() { return _owned_sfml_window; }
	};

	/*SFML_HCI::*/Window& window() override { return _main_window; }

	SFML_HCI(const Config& cfg)
		: _main_window(cfg.WINDOW_WIDTH, cfg.WINDOW_HEIGHT, cfg.window_title)

		//!!??	For SFML + OpenGL mixed mode (https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):
		//!!??
		//sf::glEnable(sf::GL_TEXTURE_2D); //!!?? why is this needed, if SFML already draws into an OpenGL canvas?!
		//!!??	--> https://en.sfml-dev.org/forums/index.php?topic=11967.0
	{
	}

//	static UI& create(Config& cfg);

private:	
	/*SFML_HCI::*/Window _main_window; // So, not just a ref.

}; // class SFML_HCI

/*
static HCI& SFML_HCI::create(Config& cfg)
{
	static SFML_HCI hci_sfml(cfg);
	return hci_sfml;
}
*/

} // namespace Szim

#endif // _SA37H6FGFGH50V6BC2370_
