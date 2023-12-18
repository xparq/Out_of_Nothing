#include "OONRender_sfml.hpp"

#include "Engine/View/OrthoZoomCamera.hpp"
#include "Engine/View/ScreenView.hpp"

#include "Engine/SimApp.hpp" //!!This stinks: should only use the Model,
                        //!!perhaps *some* of the _generic_ OON stuff,
                        //!!and either have or get (as params) any gfx infrastr. resources directly!

//!! This "backend tunelling" is so sad this way"... See notes in OON_sfml!
#include "Engine/Backend/_adapter_switcher.hpp"
#include SWITCHED(BACKEND, _Backend.hpp)
#define SFML_WINDOW(game) (((SFML_Backend&)game.backend).SFML_window())


#include "Model/Math/Vector2.hpp"
#include "adapter/SFML/vector.hpp"

#include "sfw/GUI.hpp"  // Theme.hpp is not enough, it doesn't include sfw::Text!

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#include <memory>
	using std::make_shared;
#include <cassert>
//#include <iostream>
//	using std::cerr;


using namespace Szim;

namespace OON {

//----------------------------------------------------------------------------
void Renderer_SFML::reset()
{
	shapes_to_change.clear();
	shapes_to_draw.clear();
}

//----------------------------------------------------------------------------
void Renderer_SFML::render(SimApp& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// Shape indexes must be the same as the corresponding entity indexes!
	for (size_t i = 0; i < shapes_to_change.size(); ++i) {
		auto& body = game.world().bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(shapes_to_change[i]);
		shape->setFillColor(sf::Color((body->color << 8) | p_alpha));

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

		//!! The size and coords. of the screen view pane (UI viewport) are NOT directly
		//!! related to the camera view, but would obviously be best if they were identical!...

	// a)
		auto vpos = game.main_view().camera()
			.world_to_view_coord(body->p - Math::Vector2f(body->r, -body->r)); //!! Rely on the objects' own origin offset instead!
			                                                                   //!! Mind the inverted camera & model y, too!
	// b)
	//	Szim::View::OrthoZoomCamera& oon_camera = (Szim::View::OrthoZoomCamera&) game.main_view().camera();
	//	auto vpos = oon_camera.world_to_view_coord(body->p - Math::Vector2f(body->r, -body->r)); //!! Rely on the objects' own origin offset instead!
	//	                                                                                         //!! Mind the inverted camera & model y, too!
		//!! Which they currently are NOT... The vertical axis (y) of the camera view is
		//!! a) inverted wrt. SFML (draw) coords., b) its origin is the center of the camera view.
		//!! -> #221, #445
		trshape.setPosition({ vpos.x + float(game.backend.hci.window().width/2),
			             -vpos.y + float(game.backend.hci.window().height/2)}); //!! "Standardize" on the view's centered origin instead!

//cerr << "render(): shape.setPos -> x = " << oon_camera.cfg.width /2 + (body->p.x - body->r) * oon_camera.scale() + oon_camera.offset.x
//			       << ", y = " << oon_camera.cfg.height/2 + (body->p.y - body->r) * oon_camera.scale() + oon_camera.offset.y <<'\n';
	}
}

//----------------------------------------------------------------------------
void Renderer_SFML::draw(SimApp& game)
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// Grid lines...
	static sf::Color hair_color{0x44444488};
	if (const auto& camera = (const Szim::View::OrthoZoomCamera&) game.main_view().camera();
	    camera.cfg.gridlines) {
		float min_x = 0, max_x = float(game.backend.hci.window().width);
		float min_y = 0, max_y = float(game.backend.hci.window().height);
		auto [vx, vy] = camera.grid_offset();//!!?? + Math::Vector2f{max_x/2, max_y/2};
		vx += max_x/2;
		vy = max_y/2 - vy;

		sf::Vertex vhair[] = {{{vx, min_y}, hair_color}, {{vx, max_y}, hair_color}};
		sf::Vertex hhair[] = {{{min_x, vy}, hair_color}, {{max_x, vy}, hair_color}};
		SFML_WINDOW(game).draw(vhair, 2, sf::PrimitiveType::Lines);
		SFML_WINDOW(game).draw(hhair, 2, sf::PrimitiveType::Lines);

/* Just do double-check the default SFML draw coords.:
sf::Vertex vcenterline[] = {{{max_x/2, min_y}, sf::Color::Black}, {{max_x/2, max_y}, sf::Color(0x88888844)}}; //!!?? WTF: no
sf::Vertex hcenterline[] = {{{min_x, max_y/2}, sf::Color::Black}, {{max_x, max_y/2}, sf::Color(0x88888844)}}; //!!?? alpha?! :-o
SFML_WINDOW(game).draw(vcenterline, 2, sf::PrimitiveType::Lines);
SFML_WINDOW(game).draw(hcenterline, 2, sf::PrimitiveType::Lines);
*/
	}

	for (const auto& entity : shapes_to_draw) {
		SFML_WINDOW(game).draw(*entity);
	}

	if (game.paused()) {
		draw_paused_banner(game);
	}
}

//----------------------------------------------------------------------------
//!!MOVE TO UI::Widget::Notice!
void Renderer_SFML::draw_paused_banner(SimApp& game)
{
	if (!sfw::Theme::loadFont(game.cfg.asset_dir + game.cfg.default_font_file)) {
		//! SFML does print errors to the console.
		return;
	}

	auto TXT_WIDTH = 300u;
	auto TXT_HEIGHT = 80u;
	sfw::Text banner("PAUSED", TXT_HEIGHT);
	banner.setPosition({
		(float)SFML_WINDOW(game).getSize().x/2 - TXT_WIDTH/2,
		(float)SFML_WINDOW(game).getSize().y/2 - TXT_HEIGHT/2 - 16 //!!fuckup offset
	});
	banner.setStyle(sf::Text::Bold | sf::Text::Bold);
	banner.setFillColor(sf::Color(sf::Color(0xc0b0a08f)));

	SFML_WINDOW(game).draw(banner);
}

//----------------------------------------------------------------------------
void Renderer_SFML::create_cached_body_shape(const SimApp& game, const Model::World::Body& body, size_t body_ndx /*= (size_t)-1*/)
{
	// There must be objects actually added already:
	assert(game.world().bodies.size() > 0);

	//!!Can only "append" for now, so ndx must refer to the last element...
	if (body_ndx == (size_t)-1) body_ndx = game.world().bodies.size() - 1;
//	assert(body_ndx == game.world().bodies.size() - 1);

	const Szim::View::OrthoZoomCamera& oon_camera = (const Szim::View::OrthoZoomCamera&) game.main_view().camera();

	//! Not all Drawables are also Transformables! (See e.g. vertex arrays etc.)
	// (But our little ugly circles are, for now; see the assert below!)
	auto shape = make_shared<sf::CircleShape>(body.r * oon_camera.scale());
	shapes_to_draw.push_back(shape);
	shapes_to_change.push_back(shape); // "... to transform"

	assert(shapes_to_draw.size() == body_ndx + 1);
	assert(shapes_to_change.size() == body_ndx + 1);
}

//----------------------------------------------------------------------------
void Renderer_SFML::delete_cached_body_shape([[maybe_unused]] const SimApp& game, size_t body_ndx)
{
	assert(body_ndx != (size_t)-1);
	// Requires the body already been deleted from the world:
	assert(game.entity_count() == shapes_to_draw.size() - 1);
	assert(game.entity_count() == shapes_to_change.size() -1);
	// Some runtime check, too:
	if (body_ndx < shapes_to_draw.size() && body_ndx < shapes_to_change.size()) {
		shapes_to_draw.erase(shapes_to_draw.begin() + body_ndx);
		shapes_to_change.erase(shapes_to_change.begin() + body_ndx);
	}
}


//----------------------------------------------------------------------------
void Renderer_SFML::resize_objects(float factor)
{
	transform_objects([factor](sf::Transformable& shape) {
		shape.setScale(shape.getScale() * factor);
	});
}

//----------------------------------------------------------------------------
void Renderer_SFML::resize_object(size_t ndx, float factor)
{
	assert(ndx < shapes_to_change.size());
	sf::Transformable& shape = *(shapes_to_change[ndx]);
	shape.setScale(shape.getScale() * factor);
}

} // namespace OON
