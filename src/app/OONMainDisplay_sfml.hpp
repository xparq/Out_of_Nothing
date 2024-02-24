#ifndef _JD928Y67295VX346536YN2374D56025M8BY856Y0356_
#define _JD928Y67295VX346536YN2374D56025M8BY856Y0356_

#include "OONMainDisplay.hpp"

#include "OONAvatar_sfml.hpp" // for focused_entity_ndx (and, not yet, but...: app.appcfg)

// For the cached SFML shapes:
//#include <SFML/Graphics/Transformable.hpp>
//#include <SFML/Graphics/Drawable.hpp>
namespace sf { class Transformable; class Drawable; }
#include <vector>
#include <memory> // shared_ptr, unique_ptr


namespace OON {

//!!class Avatar_sfml; // Alas, not enough to fw-decl for unique_ptr (unlike shared_ptr)! :-/

class OONMainDisplay_sfml : public OONMainDisplay
{
public:
	struct OONViewConfig_sfml : OONViewConfig
	{
	};

	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------

	OONMainDisplay_sfml(class OONApp& app); //! Nice, this isn't even required to be OONApp_sfml.

	void reset(const Config* recfg = nullptr) override; // Resets things to the last cfg if null.
//	void reset(Config&& recfg) override;
//	void resize(unsigned width, unsigned height) override;


	// -------------------------------------------------------------------
	// Rendering...
	// -------------------------------------------------------------------

	void draw() override;


	// -------------------------------------------------------------------
	// App-specific features...
	// -------------------------------------------------------------------

	// SFML-specific overrides
	void create_cached_shape(const Model::World::Body& body, size_t entity_ndx) override;
	void delete_cached_shape(size_t entity_ndx) override;
	void resize_objects(float factor) override;
	void resize_object(size_t ndx, float factor) override;

	//!! Move it to the UI, FFS:
	void draw_banner(const char* text) override;


	// -------------------------------------------------------------------
	// Internals...
	// -------------------------------------------------------------------
protected:
	void render_scene(); //!!?? render_scene(some target or context or options?)

	// Note: these are templates (by the auto arg), so must be in the header!
	void transform_object(size_t ndx, const auto& op) {
		auto& trshape = dynamic_cast<sf::Transformable&>(shapes_to_change[ndx]);
		op(trshape);
	}
	void transform_objects(const auto& op) // c++20 auto lambda ref (but why the `const` required by MSVC?); https://stackoverflow.com/a/67718838/1479945
	// op = [](Transformable& shape);
	{
		for (auto& shape : shapes_to_change) {
			auto& trshape = dynamic_cast<sf::Transformable&>(*shape);
			op(trshape);
		}
	}

	const Avatar_sfml& avatar(size_t ndx = 0) const;

	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
private:
	// Two lists ("views") for the same set of shapes, typed differently for
	// convenience, in order to:
	// - iterate for updates, and
	// - iterate for drawing.
	// (The common ancestor sf::Shape would be way too restritive alone, and
	// the two lists may also diverge in the future.)
	std::vector< std::shared_ptr<sf::Drawable> >      shapes_to_draw;
	std::vector< std::shared_ptr<sf::Transformable> > shapes_to_change;

	std::vector< std::unique_ptr<Avatar_sfml> > _avatars;

}; // class OONMainDisplay_sfml

} // namespace OON

#endif // _JD928Y67295VX346536YN2374D56025M8BY856Y0356_
