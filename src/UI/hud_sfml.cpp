#include "hud_sfml.hpp"

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

//----------------------------------------------------------------------------
void HUD_SFML::_setup(sf::RenderWindow& window)
{
	if (!font.loadFromFile(CFG_HUD_FONT_PATH)) {
		//! SFML does print errors to the console.
		active(false);
	}

	// Adjust for negative "virtual" offsets:
	sf::Vector2u winsize = window.getSize();

	_panel_left = req_panel_left < 0 ? winsize.x + req_panel_left : req_panel_left;
	_panel_top  = req_panel_top  < 0 ? winsize.y + req_panel_top  : req_panel_top;
}


//----------------------------------------------------------------------------
void HUD_SFML::append_line(const char* str)
{
	elements.emplace_back(font, str, DEFAULT_LINE_HEIGHT);
	auto& line = elements[line_count()-1];
	line.setPosition({
			(float)_panel_left + DEFAULT_PADDING,
			(float)_panel_top  + DEFAULT_PADDING + (line_count()-1) * DEFAULT_LINE_HEIGHT});

//	line.setStyle(sf::Text::Bold | sf::Text::Underlined);
	line.setFillColor(sf::Color(_fgcolor));
}

//----------------------------------------------------------------------------
void HUD_SFML::draw(sf::RenderWindow& window)
{
	if (!active()) return;

    //https://en.sfml-dev.org/forums/index.php?topic=25552.0
//!!Why the offset?!
//!!	tgui::Clipping clipview(window, sf::RenderStates::Default, //!!??
//!!	                  {(float)_panel_left, (float)_panel_top}, {200.f, 200.f});

//	auto saved_view = window.getView();
/*!!EMPTY... :-/ :
	sf::View vw;//({(float)_panel_left, (float)_panel_top}, {1200.f, 1200.f});
	vw.setViewport(sf::FloatRect({(float)_panel_left, (float)_panel_top}, {1.f, 1.f}));
	window.setView(vw);
!!*/
	clear_content();
	std::stringstream ss;
	for (const WATCHER& x : watchers) ss << x;
	//!! Each element goes to a new line currently! :-/
	for (std::string line; std::getline(ss, line);) {
		append_line(line.c_str());
	}

	// OK, finally draw something...
	sf::RectangleShape rect({450, //!!... "Fit-to-text" feature by recompilation ;)
		(float)line_count() * DEFAULT_LINE_HEIGHT + 2*DEFAULT_PADDING});//!! 0 for now: {(float)_panel_width, (float)_panel_height)};
	rect.setPosition({(float)_panel_left, (float)_panel_top});
	rect.setFillColor(sf::Color(_bgcolor));
	rect.setOutlineColor(sf::Color((uint32_t)(_bgcolor * 1.5f)));
	rect.setOutlineThickness(1);
	window.draw(rect);

	for (auto& text : elements) {
		window.draw(text);
	}

//	window.setView(saved_view);
}

//----------------------------------------------------------------------------
void HUD_SFML::onResize(sf::RenderWindow& window)
{
	_setup(window);
}
