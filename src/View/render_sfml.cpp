#include "View/render_sfml.hpp"

#include "Model/Math/Vector2.hpp"
#include "adapter/SFML/vector.hpp"

#include "sfw/GUI.hpp"  // Theme.hpp is not enough, it doesn't include sfw::Text!

#include "OON_sfml.hpp" //!!This stinks: should only use the Model,
                        //!!perhaps *some* of the _generic_ OON stuff,
                        //!!and either have or get (as params) any gfx infrastr. resources directly!

#include <SFML/Graphics/CircleShape.hpp>

#include <memory>
	using std::make_shared;
#include <cassert>
//#include <iostream>
//	using std::cerr;


namespace View {

//----------------------------------------------------------------------------
void Renderer_SFML::reset()
{
	shapes_to_change.clear();
	shapes_to_draw.clear();
}

//----------------------------------------------------------------------------
void Renderer_SFML::render(OON_sfml& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// The indexes match the corresponding physical bodies!
	for (size_t i = 0; i < shapes_to_change.size(); ++i) {
		auto& body = game.world().bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(shapes_to_change[i]);
		shape->setFillColor(sf::Color((body->color << 8) | p_alpha));

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

//cerr << "shape.setPos -> x = " << VIEWPORT_WIDTH/2 + (body->p.x - body->r) * game.view.zoom + game.view.offset.x
//			         << ", y = " << VIEWPORT_WIDTH/2 + (body->p.y - body->r) * game.view.zoom + game.view.offset.y <<endl;
//		auto vpos = game.view.world_to_view_coord(body->p.x - body->r, body->p.y - body->r); //!! Make the centered origin an implicit default!
//		trshape.setPosition({vpos.x + sf::Vector2f(VIEWPORT_WIDTH/2, vpos.y + VIEWPORT_HEIGHT/2)); //!! Make the centered origin an implicit default!
		auto vpos = game.view.world_to_view_coord(body->p - Math::Vector2f(body->r, body->r)); //!! Make the centered origin an implicit default!
		trshape.setPosition(to_sfVector2(vpos) + sf::Vector2f(VIEWPORT_WIDTH/2, VIEWPORT_HEIGHT/2)); //!! Make the centered origin an implicit default!
	}
}

//----------------------------------------------------------------------------
void Renderer_SFML::draw(OON_sfml& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	for (const auto& entity : shapes_to_draw) {
		game.window.draw(*entity);
	}

	if (game.paused()) {
		draw_paused_banner(game);
	}
}

//----------------------------------------------------------------------------
//!!MOVE TO UI::Widget::Notice!
void Renderer_SFML::draw_paused_banner(OON_sfml& game)
{
	if (!sfw::Theme::loadFont(game.cfg.asset_dir + "font/default.font")) {
		//! SFML does print errors to the console.
		return;
	}

	auto TXT_WIDTH = 300u;
	auto TXT_HEIGHT = 80u;
	sfw::Text banner("PAUSED", TXT_HEIGHT);
	banner.setPosition({
		(float)game.window.getSize().x/2 - TXT_WIDTH/2,
		(float)game.window.getSize().y/2 - TXT_HEIGHT/2 - 16 //!!fuckup offset
	});
	banner.setStyle(sf::Text::Bold | sf::Text::Bold);
	banner.setFillColor(sf::Color(sf::Color(0xc0b0a08f)));

	game.window.draw(banner);
}

//----------------------------------------------------------------------------
void Renderer_SFML::create_cached_body_shape(const OON_sfml& game, const Model::World::Body& body, size_t body_ndx /*= (size_t)-1*/)
{
	// There must be objects actually added already:
	assert(game.world().bodies.size() > 0);

	//!!Can only "append" for now, so ndx must refer to the last element...
	if (body_ndx == (size_t)-1) body_ndx = game.world().bodies.size() - 1;
//	assert(body_ndx == game.world().bodies.size() - 1);

	//! Not all Drawables are also Transformables! (See e.g. vertex arrays etc.)
	// (But our little ugly circles are, for now; see the assert below!)
	auto shape = make_shared<sf::CircleShape>(body.r * game.view.zoom);
	shapes_to_draw.push_back(shape);
	shapes_to_change.push_back(shape); // "... to transform"

	assert(shapes_to_draw.size() == body_ndx + 1);
	assert(shapes_to_change.size() == body_ndx + 1);
}

//----------------------------------------------------------------------------
void Renderer_SFML::delete_cached_body_shape(const OON_sfml& game, size_t body_ndx)
{game;
	assert(body_ndx != (size_t)-1);
	// Assume the body has already been deleted from the world:
	assert(shapes_to_draw.size()   == game.world().bodies.size() + 1);
	assert(shapes_to_change.size() == game.world().bodies.size() + 1);

	// Well, yep, these are runtime check, too. The related asserts are there for the debug messages only.
	if (body_ndx < shapes_to_draw.size() && body_ndx < shapes_to_change.size()) {
		shapes_to_draw.erase(shapes_to_draw.begin() + body_ndx);
		shapes_to_change.erase(shapes_to_change.begin() + body_ndx);
	}
}

} // namespace View
