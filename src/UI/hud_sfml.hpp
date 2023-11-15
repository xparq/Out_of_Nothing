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
	static constexpr uint32_t DEFAULT_TEXT_COLOR = 0x72c0c0ff; // RGBA
	static constexpr uint32_t DEFAULT_BACKGROUND_COLOR = 0x00406050;

	void clear_content() { elements.clear(); }
	auto line_count() const { return elements.size(); }
	void append_line(const char* str);
	void draw(sf::RenderWindow& window);
	void _setup(sf::RenderWindow& window);

	virtual void onResize(sf::RenderWindow& window);

public:
	HUD_SFML(sf::RenderWindow& window, const std::string& font_file,
		int xpos = DEFAULT_PANEL_LEFT, int ypos = DEFAULT_PANEL_TOP,
		uint32_t fgcolor = DEFAULT_TEXT_COLOR, uint32_t bgcolor = DEFAULT_BACKGROUND_COLOR);

protected:
	std::string _font_file;
	std::vector<sf::Text> elements; // One per line, currently!
	sf::Font font;

	int req_panel_top;
	int req_panel_left;

	int _panel_top;  // calc. by _setup()
	int _panel_left; // calc. by _setup()
	size_t _panel_width  = DEFAULT_PANEL_WIDTH;
	size_t _panel_height = DEFAULT_PANEL_HEIGHT;

	uint32_t _fgcolor = DEFAULT_TEXT_COLOR;
	uint32_t _bgcolor = DEFAULT_BACKGROUND_COLOR;
};

}; // namespace
#endif // _M09827N4536R87B4M9O8HG_
