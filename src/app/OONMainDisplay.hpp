#ifndef _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
#define _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_

#include "Szim/View/ScreenView.hpp"
#include "Szim/View/OrthoZoomCamera.hpp"

#include "Szim/Model/Meta.hpp"
namespace OON::Model { struct Entity; } // #include "app/Model/Entity.hpp"

#include <cstdint>

//!! Would only be needed for app() nicely not returning OONApp, just SzimApp,
//!! BUT...: WE KNOW NEITHER of them here (i.e. their inh. chain), so can't upcast _app!... :-/
//!!namespace Szim { class SimApp; }

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

//!!	Szim::SimApp&   app() const /*!!??override??!!*/ { return _app; } //!! See the comment at the includes why this doesn't work (compile)!...
	const auto&     app() const { return static_cast<const  OON::OONApp&>(_app); }
	const auto& oon_app() const { return static_cast<const  OON::OONApp&>(app()); }

	// -------------------------------------------------------------------
	// App-specific features...
	// -------------------------------------------------------------------

	      auto& oon_camera()       { return (      MainCameraType&) camera(); }
	const auto& oon_camera() const { return (const MainCameraType&) camera(); }

	void dim()   { p_alpha = ALPHA_INACTIVE; }
	void undim() { p_alpha = ALPHA_ACTIVE; }

	// -------------------------------------------------------------------
	// Pure virtuals for the actual drawing impl...
	virtual void create_cached_shape(const Model::Entity& body, Szim::Model::EntityID entity_ndx) = 0;
	virtual void delete_cached_shape(Szim::Model::EntityID entity_ndx) = 0;
	virtual void resize_objects(float factor) = 0;
	virtual void resize_object(Szim::Model::EntityID ndx, float factor) = 0;

	//!! Sigh... Move this to the UI already:
	virtual void draw_banner(const char* text) const = 0;

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
