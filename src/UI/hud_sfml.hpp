#ifndef _M09827N4536R87B4M9O8HG_
#define _M09827N4536R87B4M9O8HG_

#include "hud.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <vector>
	//using std::vector;
#include <typeinfo>
#include <tuple>
	//using std::tuple, std::make_tuple;
#include <any>
	//using std::any;
#include <functional>
	//using std::function;
#include <string>
	//using std::string;
//!!#include <string_view>
//!!	//using std::string_view;
#include <utility>
	//using std::exchange;


namespace UI {

//----------------------------------------------------------------------------
struct HUD_SFML : public HUD
{
	void _setup(sf::RenderWindow& window);

	void renderstate_clear() { lines.clear(); }
	void renderstate_append_line(const std::string& str);
	auto renderstate_line_count() const { return lines.size(); }
	void draw(sf::RenderWindow& window);

	virtual void onResize(sf::RenderWindow& window);

public:
	HUD_SFML(sf::RenderWindow& window, const Config& cfg);

protected:
	Config cfg;

	std::string _font_file;
	std::vector<sf::Text> lines;
	sf::Font font;

	int      _panel_left; // calc. by _setup()
	int      _panel_top;  // calc. by _setup()
	unsigned _panel_width  = DEFAULT_PANEL_WIDTH;
	unsigned _panel_height = DEFAULT_PANEL_HEIGHT;
};

}; // namespace
#endif // _M09827N4536R87B4M9O8HG_
