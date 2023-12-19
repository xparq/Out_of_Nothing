#ifndef _JD928Y67295VX346536YN2374D56025M8BY856Y0356_
#define _JD928Y67295VX346536YN2374D56025M8BY856Y0356_

#include "OONMainDisplay.hpp"

// For the cached SFML shapes:
//#include <SFML/Graphics/Transformable.hpp>
//#include <SFML/Graphics/Drawable.hpp>
namespace sf { class Transformable; class Drawable; }
#include <vector>
#include <memory> // shared_ptr


namespace OON {

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
	//!! Just directly moved from the legacy renderer for now:
	void render(); //!!?? render(some target or context or options?) and is it worth separating from draw()?

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

	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
private:
	//!! Forgot why the two sets! Explain!!!
	std::vector< std::shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	//!!misnomer alert below! should be sg. like "body_images" (as they are not just any Transformables!!! -- these are linked to model entities!):
	std::vector< std::shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

}; // class OONMainDisplay_sfml

} // namespace OON

#endif // _JD928Y67295VX346536YN2374D56025M8BY856Y0356_
