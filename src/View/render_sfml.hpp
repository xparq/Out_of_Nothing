#ifndef _RENDER_SFML_HPP_
#define _RENDER_SFML_HPP_

//!!#include "Engine/Renderer.hpp"

#include "Model/World.hpp"

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>

#include <vector>
#include <memory> // shared_ptr


namespace Szim {
class SimApp; //!! Sigh... Should really only depend on the generic app class,
                //!! but then parallell polymorphic derivation (of both the app
                //!! and the renderer) is a nightmare, involving also a Platform
		//!! abstraction (or at least a Window -> Window_SFML wrapper)...
}
namespace View {

//----------------------------------------------------------------------------
class Renderer
{
public:
	virtual void reset() = 0;
};


//----------------------------------------------------------------------------
class Renderer_SFML : public Renderer// "View"
{
public:
//!!->cfg:
//!!	constexpr static auto WINDOW_WIDTH  = 1024;
//!!	constexpr static auto WINDOW_HEIGHT = 768;

	// Size of the view pane (viewport); NOT directly related to Model::View, but
	// should obviously be mapped directly to display (viewport) coords, if at all possible.

	constexpr static auto ALPHA_ACTIVE = 255;
	constexpr static auto ALPHA_INACTIVE = 127;

// Ops
	void render(Szim::SimApp& game); // can't keep it inline here: uses the game object!
	void draw(Szim::SimApp& game); // can't keep it inline here: uses the game object!
	void draw_paused_banner(Szim::SimApp& game);

	void create_cached_body_shape(const Szim::SimApp& game, const Model::World::Body& body, size_t body_ndx = (size_t)-1); //!!that -1, ugh... sorry! ;)
	void delete_cached_body_shape(const Szim::SimApp& game, size_t body_ndx);

// Internals
//!!protected: //!!STILL USED DIRECTLY BY THE GAME CONTROLLER CLASS! :-/
	void resize_objects(float factor);

	void transform_objects(const auto& op) // c++20 auto lambda ref (but why the `const` required by MSVC?); https://stackoverflow.com/a/67718838/1479945
	// op = [](Transformable& shape);
	{
		for (auto& shape : shapes_to_change) {
			auto& trshape = dynamic_cast<sf::Transformable&>(*shape);
			op(trshape);
		}
	}

	void reset() override;

// Housekeeping
	Renderer_SFML() = default;

// Rendering params:
	uint8_t p_alpha = ALPHA_ACTIVE;

// Internals:
protected:
	std::vector< std::shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	//!!misnomer alert below! should be sg. like "body_images" (as they are not just any Transformables!!! -- these are linked to physical bodies!):
	std::vector< std::shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

}; // class Renderer_SFML

} // namespace View
#endif // _RENDER_SFML_HPP_
