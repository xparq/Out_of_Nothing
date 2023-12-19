#include "OONRender_sfml.hpp"

#include "Engine/View/OrthoZoomCamera.hpp"
#include "Engine/View/ScreenView.hpp"

#include "Engine/SimApp.hpp" //!!This stinks: should only use the Model,
                        //!!perhaps *some* of the _generic_ OON stuff,
                        //!!and either have or get (as params) any gfx infrastr. resources directly!

//!! This "backend tunelling" is so sad this way"... See notes in OON_sfml!
#include "Engine/Backend/_adapter_switcher.hpp"
#include SWITCHED(BACKEND, _Backend.hpp)
#define SFML_WINDOW(game) (((SFML_Backend&)game.backend).SFML_window())


#include "Model/Math/Vector2.hpp"
#include "adapter/SFML/vector.hpp"

#include "sfw/GUI.hpp"  // Theme.hpp is not enough, it doesn't include sfw::Text!

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#include <memory>
	using std::make_shared;
#include <cassert>
//#include <iostream>
//	using std::cerr;


using namespace Szim;

namespace OON {

//----------------------------------------------------------------------------
void Renderer_SFML::reset()
{
	shapes_to_change.clear();
	shapes_to_draw.clear();
}

//----------------------------------------------------------------------------
void Renderer_SFML::draw(SimApp& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// Grid lines...
	static sf::Color hair_color{0x44444488};
	if (const auto& camera = (const Szim::View::OrthoZoomCamera&) game.main_view().camera();
	    camera.cfg.gridlines) {
		float min_x = 0, max_x = float(game.backend.hci.window().width);
		float min_y = 0, max_y = float(game.backend.hci.window().height);
		auto [vx, vy] = camera.grid_offset();//!!?? + Math::Vector2f{max_x/2, max_y/2};
		vx += max_x/2;
		vy = max_y/2 - vy;

		sf::Vertex vhair[] = {{{vx, min_y}, hair_color}, {{vx, max_y}, hair_color}};
		sf::Vertex hhair[] = {{{min_x, vy}, hair_color}, {{max_x, vy}, hair_color}};
		SFML_WINDOW(game).draw(vhair, 2, sf::PrimitiveType::Lines);
		SFML_WINDOW(game).draw(hhair, 2, sf::PrimitiveType::Lines);

/* Just do double-check the default SFML draw coords.:
sf::Vertex vcenterline[] = {{{max_x/2, min_y}, sf::Color::Black}, {{max_x/2, max_y}, sf::Color(0x88888844)}}; //!!?? WTF: no
sf::Vertex hcenterline[] = {{{min_x, max_y/2}, sf::Color::Black}, {{max_x, max_y/2}, sf::Color(0x88888844)}}; //!!?? alpha?! :-o
SFML_WINDOW(game).draw(vcenterline, 2, sf::PrimitiveType::Lines);
SFML_WINDOW(game).draw(hcenterline, 2, sf::PrimitiveType::Lines);
*/
	}

	for (const auto& entity : shapes_to_draw) {
		SFML_WINDOW(game).draw(*entity);
	}

	if (game.paused()) {
		draw_paused_banner(game);
	}
}

//----------------------------------------------------------------------------
//!!MOVE TO UI::Widget::Notice!
void Renderer_SFML::draw_paused_banner(SimApp& game)
{
	if (!sfw::Theme::loadFont(game.cfg.asset_dir + game.cfg.default_font_file)) {
		//! SFML does print errors to the console.
		return;
	}

	auto TXT_WIDTH = 300u;
	auto TXT_HEIGHT = 80u;
	sfw::Text banner("PAUSED", TXT_HEIGHT);
	banner.setPosition({
		(float)SFML_WINDOW(game).getSize().x/2 - TXT_WIDTH/2,
		(float)SFML_WINDOW(game).getSize().y/2 - TXT_HEIGHT/2 - 16 //!!fuckup offset
	});
	banner.setStyle(sf::Text::Bold | sf::Text::Bold);
	banner.setFillColor(sf::Color(sf::Color(0xc0b0a08f)));

	SFML_WINDOW(game).draw(banner);
}

//----------------------------------------------------------------------------
void Renderer_SFML::create_cached_body_shape(const SimApp& game, const Model::World::Body& body, size_t body_ndx /*= (size_t)-1*/)
{
	// There must be objects actually added already:
	assert(game.world().bodies.size() > 0);

	//!!Can only "append" for now, so ndx must refer to the last element...
	if (body_ndx == (size_t)-1) body_ndx = game.world().bodies.size() - 1;
//	assert(body_ndx == game.world().bodies.size() - 1);

	const Szim::View::OrthoZoomCamera& oon_camera = (const Szim::View::OrthoZoomCamera&) game.main_view().camera();

	//! Not all Drawables are also Transformables! (See e.g. vertex arrays etc.)
	// (But our little ugly circles are, for now; see the assert below!)
	auto shape = make_shared<sf::CircleShape>(body.r * oon_camera.scale());
	shapes_to_draw.push_back(shape);
	shapes_to_change.push_back(shape); // "... to transform"

	assert(shapes_to_draw.size() == body_ndx + 1);
	assert(shapes_to_change.size() == body_ndx + 1);
}

//----------------------------------------------------------------------------
void Renderer_SFML::delete_cached_body_shape([[maybe_unused]] const SimApp& game, size_t body_ndx)
{
	assert(body_ndx != (size_t)-1);
	// Requires the body already been deleted from the world:
	assert(game.entity_count() == shapes_to_draw.size() - 1);
	assert(game.entity_count() == shapes_to_change.size() -1);
	// Some runtime check, too:
	if (body_ndx < shapes_to_draw.size() && body_ndx < shapes_to_change.size()) {
		shapes_to_draw.erase(shapes_to_draw.begin() + body_ndx);
		shapes_to_change.erase(shapes_to_change.begin() + body_ndx);
	}
}


//----------------------------------------------------------------------------
void Renderer_SFML::resize_objects(float factor)
{
	transform_objects([factor](sf::Transformable& shape) {
		shape.setScale(shape.getScale() * factor);
	});
}

//----------------------------------------------------------------------------
void Renderer_SFML::resize_object(size_t ndx, float factor)
{
	assert(ndx < shapes_to_change.size());
	sf::Transformable& shape = *(shapes_to_change[ndx]);
	shape.setScale(shape.getScale() * factor);
}

} // namespace OON
