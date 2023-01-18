#include "render-sfml.hpp"
#include "engine-sfml.hpp"

void Render_SFML::render_next_frame(const Engine_SFML& game) //! Can't stay in the header: uses the Engine!
// Should be idempotent -- which doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// The indexes match the corresponding physical bodies!
	for (size_t i = 0; i < shapes_to_change.size(); ++i)
	{
		auto& body = game.world.bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(shapes_to_change[i]);
		shape->setFillColor(sf::Color(70 + body->color, 12, 50 - body->color, p_alpha));

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

//cerr << "shape.setPos -> x = " << VIEW_WIDTH/2  + (body->p.x - body->r) * game._SCALE + game._OFFSET_X
//			       << ", y = " << VIEW_HEIGHT/2 + (body->p.y - body->r) * game._SCALE + game._OFFSET_Y <<endl;
		trshape.setPosition(sf::Vector2f(
			VIEW_WIDTH/2  + (body->p.x - body->r) * game._SCALE + game._OFFSET_X,
			VIEW_HEIGHT/2 + (body->p.y - body->r) * game._SCALE + game._OFFSET_Y));
	}
}
