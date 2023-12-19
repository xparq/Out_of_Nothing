#include "OONMainDisplay_sfml.hpp"

//!! Keep it as tight as possible:
//#include "OON_sfml.hpp"
//#include "OON.hpp"
#include "Engine/SimApp.hpp"

#include <iostream> //!! DEBUG
	using std::cerr;


namespace OON {

/*
OONMainDisplay::OONMainDisplay(Config cfg) : _cfg(cfg)
{
	reset(); // Calc. initial state
}
*/

OONMainDisplay_sfml::OONMainDisplay_sfml(OON_sfml& app)
	: OONMainDisplay({
		//!!
		//!! OONViewConfig init...
		//!!
	}, (OONApp&)app) //!!?? WhyTF wouldn't this be auto-converted?! :-o
{
//	reset(); // Calc. initial state
cerr <<	"DBG> OONMainDisplay ctor: camera pointer is now: " << _camera << "\n";
}

void OONMainDisplay_sfml::reset(const Config* recfg)
{
	if (recfg) _cfg = *recfg;

	resize(_cfg.width, _cfg.height);

	// Recreate the shapes...
	renderer.reset();
	for (size_t n = 0; n < app().const_world().bodies.size(); ++n) {
		renderer.create_cached_body_shape(app(), *(app().const_world().bodies[n]), n); // * for smart_ptr
	}


}

/*
void OONMainDisplay_sfml::reset(Config&& recfg) { reset(&recfg); }

void OONMainDisplay_sfml::resize(unsigned width, unsigned height)
{
	_cfg.width  = width;
	_cfg.height = height;
}
*/





//----------------------------------------------------------------------------
void OONMainDisplay_sfml::create_cached_body_shape(const Model::World::Body& body, size_t entity_ndx) //override
{
	renderer.create_cached_body_shape(app(), body, entity_ndx);
}
void OONMainDisplay_sfml::delete_cached_body_shape(size_t entity_ndx) //override
{
	renderer.delete_cached_body_shape(app(), entity_ndx);
}
void OONMainDisplay_sfml::resize_objects(float factor) //override
{
	renderer.resize_objects(factor);
}
void OONMainDisplay_sfml::resize_object(size_t ndx, float factor) //override
{
	renderer.resize_object(ndx, factor);
}




//----------------------------------------------------------------------------
//!! Just directly moved from the legacy renderer for now:
void OONMainDisplay_sfml::render()
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// Shape indexes must be the same as the corresponding entity indexes!
	for (size_t i = 0; i < renderer.shapes_to_change.size(); ++i) {
		auto& body = app().world().bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(renderer.shapes_to_change[i]);
		shape->setFillColor(sf::Color((body->color << 8) | p_alpha));

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

		//!! The size and coords. of the screen view pane (UI viewport) are NOT directly
		//!! related to the camera view, but would obviously be best if they were identical!...

	// a)
		auto vpos = app().main_view().camera()
			.world_to_view_coord(body->p - Math::Vector2f(body->r, -body->r)); //!! Rely on the objects' own origin offset instead!
			                                                                   //!! Mind the inverted camera & model y, too!
	// b)
	//	Szim::View::OrthoZoomCamera& oon_camera = (Szim::View::OrthoZoomCamera&) game.main_view().camera();
	//	auto vpos = oon_camera.world_to_view_coord(body->p - Math::Vector2f(body->r, -body->r)); //!! Rely on the objects' own origin offset instead!
	//	                                                                                         //!! Mind the inverted camera & model y, too!
		//!! Which they currently are NOT... The vertical axis (y) of the camera view is
		//!! a) inverted wrt. SFML (draw) coords., b) its origin is the center of the camera view.
		//!! -> #221, #445
		trshape.setPosition({ vpos.x + float(app().backend.hci.window().width/2),
			             -vpos.y + float(app().backend.hci.window().height/2)}); //!! "Standardize" on the view's centered origin instead!

//cerr << "render(): shape.setPos -> x = " << oon_camera.cfg.width /2 + (body->p.x - body->r) * oon_camera.scale() + oon_camera.offset.x
//			       << ", y = " << oon_camera.cfg.height/2 + (body->p.y - body->r) * oon_camera.scale() + oon_camera.offset.y <<'\n';
	}
}



void OONMainDisplay_sfml::draw() // override
{
	render();
	renderer.draw((Szim::SimApp&)app()); //!!?? render(some target or context or options?) and is it worth separating from draw()?
}

} // namespace OON
