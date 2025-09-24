#include "hud_sfml.hpp"

#include "myco/abstract/geometry/Rectangle.hpp"
#include "myco/abstract/math/Vector.hpp"
#include "myco/gfx/element/FilledRect.hpp"
#include "myco/style/Color.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

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
#include <algorithm>
	using std::max;
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
	_setup(window.getSize().x, window.getSize().y);
}

//----------------------------------------------------------------------------
void HUD_SFML::_setup(unsigned width, unsigned height)
{
	if (!font.load(cfg.font_file)) {
		//! SFML has already written the error to the console.
		active(false);
	}

	// Adjust for negative "virtual" offsets:
	_panel_left = cfg.panel_left < 0 ? width  + cfg.panel_left : cfg.panel_left;
	_panel_top  = cfg.panel_top  < 0 ? height + cfg.panel_top  : cfg.panel_top;
}


//----------------------------------------------------------------------------
void HUD_SFML::renderstate_append_line(const string& str)
{
	lines.emplace_back(font, str, cfg.line_height - cfg.line_spacing);
	auto& line = lines[renderstate_line_count()-1];
	line.position({
			(float)_panel_left + DEFAULT_PADDING,
			(float)_panel_top  + DEFAULT_PADDING + (renderstate_line_count()-1) * cfg.line_height});

	myco::fRect linerect = line.size();
	myco::fVec2 size(linerect.left() * 2 + linerect.width(), linerect.top() + linerect.height());
	if (linerect.width() + 2 * DEFAULT_PADDING > _panel_width)
		_panel_width = (unsigned) linerect.width() + 2 * DEFAULT_PADDING;

//	line.setStyle(sf::Text::Bold | sf::Text::Underlined);
	line.color(cfg.fgcolor);
}

//----------------------------------------------------------------------------
void HUD_SFML::draw(sf::RenderWindow& window)
{
	if (!active()) return;

    // SFML3 has got some clipping support now! Check/use it!
   // Use it in general for every Myco widget! And DELETE THE CRUFT BELOW!
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
	myco::gfx::FilledRect rect;
	rect.size = myco::fVec2{_panel_width, renderstate_line_count() * cfg.line_height + 2 * DEFAULT_PADDING}; //!! 0 padding for now
	rect.position = myco::fVec2{_panel_left, _panel_top};
	rect.fillColor = cfg.bgcolor;

	// Add a fine border...
	// - This is still not bullet-proof, but the best way I could conjure up that automatically looks nice:
	float amp = 255.f / std::max(myco::gfx::Color(cfg.bgcolor).r(), std::max(myco::gfx::Color(cfg.bgcolor).g(), myco::gfx::Color(cfg.bgcolor).b()));
	rect.borderColor = myco::gfx::Color(
		uint8_t(myco::gfx::Color(cfg.bgcolor).r() * amp),
		uint8_t(myco::gfx::Color(cfg.bgcolor).g() * amp),
		uint8_t(myco::gfx::Color(cfg.bgcolor).b() * amp),
		uint8_t(myco::gfx::Color(cfg.bgcolor).a())
	); // + sf::Color(cfg.fgcolor).a) / 2

	rect.draw(myco::gfx::RenderContext(window));

	for (auto& text : lines) {
		window.draw(text);
	}

//	window.setView(saved_view);
}

//----------------------------------------------------------------------------
void HUD_SFML::onResize(unsigned width, unsigned height)
{
	_setup(width, height);
}
