#ifndef _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
#define _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_

#include "Engine/View/ScreenView.hpp"
#include "Engine/View/OrthoZoomCamera.hpp"

#include <cstdint>


namespace Szim::View { class Camera; }

//!!namespace Model { class World; class World::Body; } //!! *Sigh*, C++, still nope! :-o https://stackoverflow.com/a/36736618/1479945
#include "Model/World.hpp"

//#include "Model/Math/Vector2.hpp"

namespace OON {

class OONApp;

class OONMainDisplay : public Szim::View::ScreenView
{
public:
	//!!Migrating from the legacy renderer (move to the config!!):
	constexpr static uint8_t ALPHA_ACTIVE = 255;
	constexpr static uint8_t ALPHA_INACTIVE = 127;

	using MainCameraType = Szim::View::OrthoZoomCamera;
	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------

	struct OONViewConfig : Szim::View::ScreenView::Config
	{
	};

//	OONMainDisplay() {}
//	OONMainDisplay(Config cfg);
	OONMainDisplay(OONViewConfig/*& to avoid slicing; but disallow temp.? No. Slicing's OK here.*/ cfg, OONApp& app);

	auto& oon_app() { return (OONApp&)_app; }
	Szim::SimApp& app() override { return (Szim::SimApp&) oon_app(); }
	//!! These two should be the other way around later! :)

/*
	void reset(const Config* recfg = nullptr) override; // Resets things to the last cfg if null.
	void reset(Config&& recfg) override;
	void resize(unsigned width, unsigned height) override;
*/

	void dim()   { p_alpha = ALPHA_INACTIVE; }
	void undim() { p_alpha = ALPHA_ACTIVE; }

	// These are only pure virt. becasue we don't have an OON-generic, SFML-free 'renderer' member of this class... :-/
	virtual void create_cached_body_shape(const Model::World::Body& body, size_t entity_ndx) = 0;
	virtual void delete_cached_body_shape(size_t entity_ndx) = 0;
	virtual void resize_objects(float factor) = 0;
	virtual void resize_object(size_t ndx, float factor) = 0;


	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
protected:
	OONApp& _app;
	MainCameraType _oon_default_camera;

	// Rendering params:
	uint8_t p_alpha = ALPHA_ACTIVE;

}; // class OONMainDisplay

} // namespace OON

#endif // _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
