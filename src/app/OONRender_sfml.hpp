#ifndef _8467T9B84C5XC9678456Y2897WB597TY6793_
#define _8467T9B84C5XC9678456Y2897WB597TY6793_

#include "Engine/View/Renderer.hpp"

#include "Model/World.hpp"

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>

#include <vector>
#include <memory> // shared_ptr


namespace Szim { class SimApp; }
		//!! Should only depend on the generic app class,
                //!! but the parallel polymorphic derivation (of both the app
                //!! and the renderer) is a nightmare, involving also a Platform
		//!! abstraction (or at least a Window -> Window_SFML wrapper)...

namespace OON {

//----------------------------------------------------------------------------
class Renderer_SFML : public Szim::View::Renderer
{
friend class OONMainDisplay_sfml; //!! Only until #461...

public:
// Ops
	void draw(Szim::SimApp& game); // can't keep it inline here: uses the game object!
	void draw_paused_banner(Szim::SimApp& game);

	void create_cached_body_shape(const Szim::SimApp& game, const Model::World::Body& body, size_t body_ndx = (size_t)-1); //!!that -1, ugh... sorry! ;)
	void delete_cached_body_shape(const Szim::SimApp& game, size_t body_ndx);

// Internals
//!!protected: //!!STILL USED DIRECTLY BY THE GAME CONTROLLER CLASS! :-/
	void resize_objects(float factor);
	void resize_object(size_t ndx, float factor);

	void transform_objects(const auto& op) // c++20 auto lambda ref (but why the `const` required by MSVC?); https://stackoverflow.com/a/67718838/1479945
	// op = [](Transformable& shape);
	{
		for (auto& shape : shapes_to_change) {
			auto& trshape = dynamic_cast<sf::Transformable&>(*shape);
			op(trshape);
		}
	}

	void transform_object(size_t ndx, const auto& op) {
		auto& trshape = dynamic_cast<sf::Transformable&>(shapes_to_change[ndx]);
		op(trshape);
	}

	void reset() override;

// Housekeeping
	Renderer_SFML() = default;

// Internals:
protected:
	std::vector< std::shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	//!!misnomer alert below! should be sg. like "body_images" (as they are not just any Transformables!!! -- these are linked to physical bodies!):
	std::vector< std::shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

}; // class Renderer_SFML

} // namespace OON

#endif // _8467T9B84C5XC9678456Y2897WB597TY6793_
