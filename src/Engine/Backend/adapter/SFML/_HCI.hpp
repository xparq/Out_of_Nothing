#ifndef _SA37H6FGFGH50V6BC2370_
#define _SA37H6FGFGH50V6BC2370_

#include "Engine/Backend/HCI.hpp"
#include "Engine/SimAppConfig.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/System/String.hpp> //!!?? WTF is this not included already?! :-o I've never had to do this before!
//!!#include <SFML/Window/Context.hpp>
//!!#include <SFML/Window/Event.hpp>

namespace Szim {

struct SFML_HCI : HCI
{
	//--------------------------------------------------------------------
	// Abstract API Impl...
	//--------------------------------------------------------------------

	struct Window : HCI::Window
	{
		Window(const Window::Config& requested_cfg); // Polymorphic, so not by-val (albeit slicing would be kinda OK, too)

		//--------------------------------------------------------------------
		// SFML-specifics (should only be used within the adapter layer!)...
		//--------------------------------------------------------------------
		sf::RenderWindow _owned_sfml_window; //! <- Not just a ref!
		operator sf::RenderWindow&() { return _owned_sfml_window; }
	};

	SFML_HCI::Window& main_window() override { return _main_window; }
	public: void switch_fullscreen(bool fullscreen) override;
	public: void set_frame_rate_limit(unsigned fps) override;
		// SFML's window.create() will reset it, so it's implicitly saved/restored, too!

	SFML_HCI(SimAppConfig& syscfg); // Creates the SFML main window

	//--------------------------------------------------------------------
	// SFML-specifics (should only be used within the adapter layer!)...
	//--------------------------------------------------------------------
	sf::RenderWindow& SFML_window() { return main_window(); }

	//--------------------------------------------------------------------
private:
	SimAppConfig& syscfg;          // Keep a ref. to the global cfg.!
	SFML_HCI::Window _main_window; // <- Not just a ref, but the actual window!
}; // class SFML_HCI

} // namespace Szim

#endif // _SA37H6FGFGH50V6BC2370_
