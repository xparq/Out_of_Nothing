#ifndef _JD928Y67295VX346536YN2374D56025M8BY856Y0356_
#define _JD928Y67295VX346536YN2374D56025M8BY856Y0356_

#include "OONMainDisplay.hpp"

//!! #461:
#include "OONRender_sfml.hpp"

namespace OON {

class OON_sfml;


class OONMainDisplay_sfml : public OONMainDisplay
{
public:
	struct OONViewConfig_sfml : OONViewConfig
	{
	};

	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------
	OONMainDisplay_sfml(OON_sfml& app);

	auto& sfml_app() { return (OON_sfml&)_app; }

	void reset(const Config* recfg = nullptr) override; // Resets things to the last cfg if null.
//	void reset(Config&& recfg) override;
//	void resize(unsigned width, unsigned height) override;

	// SFML-specific overrides
	void create_cached_body_shape(const Model::World::Body& body, size_t entity_ndx) override;
	void delete_cached_body_shape(size_t entity_ndx) override;
	void resize_objects(float factor) override;
	void resize_object(size_t ndx, float factor) override;

	//!! Just directly moved from the legacy renderer for now:
	void render(); //!!?? render(some target or context or options?) and is it worth separating from draw()?

	void draw() override;

 //!! Eliminate! (Merge into the view impl. itself etc.) -> #461
private: OON::Renderer_SFML renderer;
}; // class OONMainDisplay_sfml

} // namespace OON

#endif // _JD928Y67295VX346536YN2374D56025M8BY856Y0356_
