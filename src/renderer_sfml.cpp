#include "renderer_sfml.hpp"
#include "engine_sfml.hpp"

#include <SFML/Graphics/CircleShape.hpp>


void Renderer_SFML::render(const Engine_SFML& game)
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

void Renderer_SFML::draw(const Engine_SFML& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	for (const auto& entity : shapes_to_draw) {
		game.window.draw(*entity);
	}
}
