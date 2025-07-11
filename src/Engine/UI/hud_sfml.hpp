#ifndef _M09827N4536R87B4M9O8HG_
#define _M09827N4536R87B4M9O8HG_

#include "hud.hpp"
#include "Engine/Config.hpp" //!! Move to szlib (#398)

#include "myco/gfx/element/Font.hpp"
#include "myco/gfx/element/Text.hpp"

//!!#include "myco/gfx/Render.hpp"
namespace sf { class RenderWindow; }

#include <vector>
#include <string>

namespace UI {

//----------------------------------------------------------------------------
class HUD_SFML : public HUD
{
public:
	void _setup(unsigned width, unsigned height);

	void renderstate_clear() { lines.clear(); }
	void renderstate_append_line(const std::string& str);
	auto renderstate_line_count() const { return lines.size(); }
//!!	void draw(myco::gfx::RenderContext& ctx);
	void draw(sf::RenderWindow& window);

	virtual void onResize(unsigned width, unsigned height) override;

public:
	HUD_SFML(sf::RenderWindow& window, const Config& cfg);

protected:
	Config cfg;

	std::string _font_file;
	std::vector<myco::gfx::Text> lines;
	myco::gfx::Font font;

	int      _panel_left; // calc. by _setup()
	int      _panel_top;  // calc. by _setup()
	unsigned _panel_width  = DEFAULT_PANEL_WIDTH;
	unsigned _panel_height = DEFAULT_PANEL_HEIGHT;
};

}; // namespace
#endif // _M09827N4536R87B4M9O8HG_
