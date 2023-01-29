#include "renderer_sfml.hpp"
#include "engine_sfml.hpp"

#include <SFML/Graphics/CircleShape.hpp>

#include <memory>
	using std::make_shared;
#include <cassert>


void Renderer_SFML::render(Engine_SFML& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// The indexes match the corresponding physical bodies!
	for (size_t i = 0; i < shapes_to_change.size(); ++i) {
		auto& body = game.world.bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(shapes_to_change[i]);
		shape->setFillColor(sf::Color((body->color << 8) | p_alpha));

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

//cerr << "shape.setPos -> x = " << VIEW_WIDTH/2  + (body->p.x - body->r) * game._SCALE + game._OFFSET_X
//			       << ", y = " << VIEW_HEIGHT/2 + (body->p.y - body->r) * game._SCALE + game._OFFSET_Y <<endl;
		trshape.setPosition(sf::Vector2f(
			VIEW_WIDTH/2  + (body->p.x - body->r) * game._SCALE + game._OFFSET_X,
			VIEW_HEIGHT/2 + (body->p.y - body->r) * game._SCALE + game._OFFSET_Y));
	}
}

void Renderer_SFML::draw(Engine_SFML& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	for (const auto& entity : shapes_to_draw) {
		game.window.draw(*entity);
	}
}


void Renderer_SFML::create_cached_body_shape(const Engine_SFML& game, const World::Body& body, size_t body_ndx /*= (size_t)-1*/)
{
	// There must be objects actually added already:
	assert(game.world.bodies.size() > 0);

	//!!Can only "append" for now, so ndx must refer to the last element...
	if (body_ndx == (size_t)-1) body_ndx = game.world.bodies.size() - 1;
	assert(body_ndx == game.world.bodies.size() - 1);

	//! Not all Drawables are also Transformables! (See e.g. vertex arrays etc.)
	// (But our little ugly circles are, for now; see the assert below!)
	auto shape = make_shared<sf::CircleShape>(body.r * game._SCALE);
	shapes_to_draw.push_back(shape);
	shapes_to_change.push_back(shape); // "... to transform"

	assert(shapes_to_draw.size() == body_ndx + 1);
	assert(shapes_to_change.size() == body_ndx + 1);
}

void Renderer_SFML::delete_cached_body_shape(const Engine_SFML& game, size_t body_ndx)
{game;
	assert(body_ndx != (size_t)-1);
	// Assume the body has already been deleted from the world:
	assert(shapes_to_draw.size()   == game.world.bodies.size() + 1);
	assert(shapes_to_change.size() == game.world.bodies.size() + 1);

	// Well, yep, these are runtime check, too. The related asserts are there for the debug messages only.
	if (body_ndx < shapes_to_draw.size() && body_ndx < shapes_to_change.size()) {
		shapes_to_draw.erase(shapes_to_draw.begin() + body_ndx);
		shapes_to_change.erase(shapes_to_change.begin() + body_ndx);
	}
}
