#ifndef __RENDERER_SFML__
#define __RENDERER_SFML__

#include "cfg.h"

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>

#include <vector>
#include <memory> // shared_ptr


class Engine_SFML;

//----------------------------------------------------------------------------
class Renderer_SFML // "View"
{
//----------------------------------------------------------------------------
public:
	static constexpr auto VIEW_WIDTH  = 800;
	static constexpr auto VIEW_HEIGHT = 600;

	static constexpr const auto ALPHA_ACTIVE = 255;
	static constexpr auto ALPHA_INACTIVE = 127;

// Input params
	uint8_t p_alpha;

// Internals:
//!!...not quite yet -- just allow access:
public:
	std::vector< std::shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	//!!misnomer alert below! should be sg. like "body_images" (as they are not just any Transformables!!! -- these are linked to physical bodies!):
	std::vector< std::shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

// Ops
	void render(const Engine_SFML& game); // can't keep it inline here: uses the Engine!
	void draw(const Engine_SFML& game); // can't keep it inline here: uses the Engine!

	void resize_objects(float factor)
	{
		transform_objects([factor](sf::Transformable& shape) {
				shape.setScale(shape.getScale() * factor);
		});
	}

	void transform_objects(const auto& op) // c++20 auto lambda ref (but why the `const` required by MSVC?); https://stackoverflow.com/a/67718838/1479945
	// op = [](Transformable& shape);
	{
		for (auto& shape : shapes_to_change) {
			auto& trshape = dynamic_cast<sf::Transformable&>(*shape);
			op(trshape);
		}
	}


// Housekeeping
	Renderer_SFML()
	      :	p_alpha(ALPHA_ACTIVE)
	{
	}
};

#endif // __RENDERER_SFML__
