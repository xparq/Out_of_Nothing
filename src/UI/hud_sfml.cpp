#include "hud_sfml.hpp"

#include "sfw/util/shim/sfml.hpp" // UTF8 conv.

#include <SFML/Graphics/RectangleShape.hpp>

//!!Struggling with doing a cliprect... (-> draw())
//!!#include <SFML/Graphics/View.hpp>
//!!#include "TGUI-Clipping.hpp"

//#include <typeinfo>
#include <string>
	using std::string, std::getline;
#include <string_view>
	using std::string_view;
#include <sstream>
	using std::stringstream;
#include <limits>
	using std::numeric_limits; // # of digits for precise float output
//#include <format>
#include <iostream>
	using std::cerr, std::endl;

using namespace std;

using namespace UI;

HUD_SFML::HUD_SFML(sf::RenderWindow& window, const Config& cfg) :
//HUD_SFML::HUD_SFML(sf::RenderWindow& window, const string& font_file,
//	int xpos, int ypos, uint32_t fgcolor, uint32_t bgcolor) :
	cfg(cfg)
{
	_setup(window);
}

//----------------------------------------------------------------------------
void HUD_SFML::_setup(sf::RenderWindow& window)
{
	if (!font.loadFromFile(cfg.font_file)) {
		//! SFML has already written the error to the console.
		active(false);
	}

	// Adjust for negative "virtual" offsets:
	sf::Vector2u winsize = window.getSize();

	_panel_left = cfg.panel_left < 0 ? winsize.x + cfg.panel_left : cfg.panel_left;
	_panel_top  = cfg.panel_top  < 0 ? winsize.y + cfg.panel_top  : cfg.panel_top;
}


//----------------------------------------------------------------------------
void HUD_SFML::renderstate_append_line(const string& str)
{
	lines.emplace_back(font, stdstring_to_SFMLString(str), cfg.line_height - cfg.line_spacing);
	auto& line = lines[renderstate_line_count()-1];
	line.setPosition({
			(float)_panel_left + DEFAULT_PADDING,
			(float)_panel_top  + DEFAULT_PADDING + (renderstate_line_count()-1) * cfg.line_height});

	sf::FloatRect linerect = line.getLocalBounds();
	sf::Vector2f size(linerect.left * 2 + linerect.width, linerect.top + linerect.height);
	if (linerect.width + 2 * DEFAULT_PADDING > _panel_width)
		_panel_width = (unsigned) linerect.width + 2 * DEFAULT_PADDING;

//	line.setStyle(sf::Text::Bold | sf::Text::Underlined);
	line.setFillColor(sf::Color(cfg.fgcolor));
}

//----------------------------------------------------------------------------
void HUD_SFML::draw(sf::RenderWindow& window)
{
	if (!active()) return;

    // SFML3 has got some clipping support now! Check/use it!
   // Use in general for every SFW widget! And DELETE THE CRUFT BELOW!
  //https://en.sfml-dev.org/forums/index.php?topic=25552.0
 //
//!!Why the offset?!
//!!	tgui::Clipping clipview(window, sf::RenderStates::Default, //!!??
//!!	                  {(float)_panel_left, (float)_panel_top}, {200.f, 200.f});

//	auto saved_view = window.getView();
/*!!EMPTY... :-/ :
	sf::View vw;//({(float)_panel_left, (float)_panel_top}, {1200.f, 1200.f});
	vw.setViewport(sf::FloatRect({(float)_panel_left, (float)_panel_top}, {1.f, 1.f}));
	window.setView(vw);
!!*/
	if (_volatile) {
		renderstate_clear();
		std::stringstream ss;
		ss << *this;

		for (std::string line; std::getline(ss, line);)
			renderstate_append_line(line);
	}

	// OK, finally draw something...
	sf::RectangleShape rect({(float)_panel_width,
		(float)renderstate_line_count() * cfg.line_height + 2*DEFAULT_PADDING});//!! 0 for now: {(float)_panel_width, (float)_panel_height)};
	rect.setPosition({(float)_panel_left, (float)_panel_top});
	rect.setFillColor(sf::Color(cfg.bgcolor));
	rect.setOutlineColor(sf::Color((uint32_t)(cfg.bgcolor * 1.5f)));
	rect.setOutlineThickness(1);
	window.draw(rect);

	for (auto& text : lines) {
		window.draw(text);
	}

//	window.setView(saved_view);
}

//----------------------------------------------------------------------------
void HUD_SFML::onResize(sf::RenderWindow& window)
{
	_setup(window);
}
