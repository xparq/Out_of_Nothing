#ifndef _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
#define _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_

#include "Szim/View/ScreenView.hpp"
#include "Szim/View/OrthoZoomCamera.hpp"

//!#include "Model/Entity.hpp"
namespace Model { struct Entity; }

#include <cstdint>


namespace OON {

class OONApp;

class OONMainDisplay : public Szim::View::ScreenView
//!
//! NOTE: A CRTP impl. would break the compilation barrier between backend-specific
//!	and "pure" code! :-/
//!
//!	template< class AppSysImpl > // CRTP for backend-specifics
//!	class OONMainDisplay : public Szim::View::ScreenView
//!
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

	OONMainDisplay(OONViewConfig/*& to avoid slicing; but disallow temp.? No. Slicing's OK here.*/ cfg, OONApp& app);

	Szim::SimApp& app() override { return (Szim::SimApp&) _app; }

	auto& oon_app() const { return (const OON::OONApp&) _app; }

	// -------------------------------------------------------------------
	// App-specific features...
	// -------------------------------------------------------------------

	const auto& oon_camera() const { return (const MainCameraType&) camera(); }
	      auto& oon_camera()       { return (      MainCameraType&) camera(); }

	void dim()   { p_alpha = ALPHA_INACTIVE; }
	void undim() { p_alpha = ALPHA_ACTIVE; }

	// -------------------------------------------------------------------
	// Pure virtuals for the actual drawing impl...
	virtual void create_cached_shape(const Model::Entity& body, Model::EntityID entity_ndx) = 0;
	virtual void delete_cached_shape(Model::EntityID entity_ndx) = 0;
	virtual void resize_objects(float factor) = 0;
	virtual void resize_object(Model::EntityID ndx, float factor) = 0;

	//!! Sigh... Move this to the UI already:
	virtual void draw_banner(const char* text) = 0;

	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
protected:
	OONApp& _app;
	MainCameraType _oon_default_camera;

	// Rendering params etc.:
	uint8_t p_alpha = ALPHA_ACTIVE;

}; // class OONMainDisplay

} // namespace OON

#endif // _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
