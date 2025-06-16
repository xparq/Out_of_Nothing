#include "OONMainDisplay_sfml.hpp"

//!! Keep it as tight as possible:
//#include "Engine/SimApp.hpp"
#include "OON.hpp" // for focused_entity_ndx (and, not yet, but...: app.appcfg)
//#include "OON_sfml.hpp"

//!! This "backend tunneling" is so sad this way"... See notes in OON_sfml.cpp!
#include "Engine/Backend/_adapter_switcher.hpp"
#include SWITCHED(BACKEND, _Backend.hpp)
#define SFML_WINDOW(app) (((Szim::SFML_Backend&)((app).backend)).SFML_window())

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <memory>
	using std::make_shared;
#include <cmath> // sin //!! Seriously, replace with a fast table lookup!
#include <cassert>
#include <iostream> //!! DEBUG
	using std::cerr;


namespace OON {

const Avatar_sfml& OONMainDisplay_sfml::avatar(size_t ndx) const
{
//!!	assert(_avatars.size());
if (!_avatars.size()) { static Avatar_sfml dummy({});
cerr <<"- INTERNAL: Hey, avatar loading must happen before other View ops.!\n";
return dummy; }

	return *_avatars[ndx > _avatars.size() - 1
	                     ? _avatars.size() - 1 : ndx];
}


//----------------------------------------------------------------------------
OONMainDisplay_sfml::OONMainDisplay_sfml(OONApp& app)
	: OONMainDisplay({
		//!!
		//!! OONViewConfig init...
		//!!
	}, app)
{
//!! This would crash ('bad alloc') with the current construction logic,
//!! i.e. no Engine (SimApp) init has been done at all yet! :-o :-/
//!!	reset(); // Calc. initial state

cerr <<	"DBG> OONMainDisplay_sfml ctor: camera pointer is now: " << _camera << "\n";
}

//----------------------------------------------------------------------------
void OONMainDisplay_sfml::reset(const Config* recfg)
{
cerr << "----------------------------------------- "<<__FUNCTION__<<"\n";

	if (recfg) _cfg = *recfg;

	resize(_cfg.width, _cfg.height);

	const auto& c_simapp = app();

	// Load avatars -- !!TESTING ONLY!!
	Avatar_sfml::prefix_path = c_simapp.cfg.asset_dir.c_str(); // Can be set per instance, too.
	for (const auto& a : c_simapp.avatars) {
		_avatars.emplace_back(std::make_unique<Avatar_sfml>(a));
// OR:
//		_avatars.emplace_back(std::make_unique<Avatar_sfml>(
//			a, Avatar_sfml::PRELOAD, c_simapp.cfg.asset_dir.c_str()
//		));
	}

	// Recreate the shapes...
	shapes_to_change.clear();
	shapes_to_draw.clear();

	for (size_t n = 0; n < c_simapp.world().bodies.size(); ++n) {
		create_cached_shape(*(c_simapp.world().bodies[n]), n); // * for smart_ptr
	}
}


//----------------------------------------------------------------------------
void OONMainDisplay_sfml::create_cached_shape(const Model::Entity& body, size_t entity_ndx) //override
{
	auto& game = app();

	// There must be objects actually added already:
	assert(game.const_world().bodies.size() > 0);

	//!!Can only "append" for now, so ndx must refer to the last element...
	if (entity_ndx == (size_t)-1) entity_ndx = game.const_world().bodies.size() - 1;
//	assert(entity_ndx == game.world().bodies.size() - 1);

	//! Not all Drawables are also Transformables! (See e.g. vertex arrays etc.)
	// (But our little ugly circles are, for now; see the assert below!)
	auto shape = make_shared<sf::CircleShape>(float(body.r) * oon_camera().scale()); //!! float hardcoded!
	shape->setOrigin({shape->getRadius(), shape->getRadius()});
	shapes_to_draw.push_back(shape);
	shapes_to_change.push_back(shape); // "... to transform"

	assert(shapes_to_draw.size()   == entity_ndx + 1);
	assert(shapes_to_change.size() == entity_ndx + 1);

//!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!! NOT HERE, NOT THIS WAY!
//!!!!!!!!!!!!!!!!!!!!!!!
	auto& player_shape = (sf::Shape&) *(shapes_to_draw[0]);
	player_shape.setTexture(&( avatar(oon_app().focused_entity_ndx).image ), true);
}

//----------------------------------------------------------------------------
void OONMainDisplay_sfml::delete_cached_shape(size_t entity_ndx) //override
{
	assert(entity_ndx != (size_t)-1);
	// Requires that the body has already been deleted from the world:
	[[maybe_unused]] auto& game = app();
	assert(game.entity_count() == shapes_to_draw.size() - 1);
	assert(game.entity_count() == shapes_to_change.size() -1);
	// Some runtime check, too:
	if (entity_ndx < shapes_to_draw.size() && entity_ndx < shapes_to_change.size()) {
		shapes_to_draw.erase(shapes_to_draw.begin() + entity_ndx);
		shapes_to_change.erase(shapes_to_change.begin() + entity_ndx);
	}
}

//----------------------------------------------------------------------------
void OONMainDisplay_sfml::resize_objects(float factor) //override
//!! Could this just call resize_object(), or that would degrade perf.?
{
	transform_objects([factor](sf::Transformable& shape) {
		shape.setScale(shape.getScale() * factor);
	});
}

//----------------------------------------------------------------------------
void OONMainDisplay_sfml::resize_object(size_t ndx, float factor) //override
{
	assert(ndx < shapes_to_change.size());
	sf::Transformable& shape = *(shapes_to_change[ndx]);
	shape.setScale(shape.getScale() * factor);
}



//----------------------------------------------------------------------------
//!! Just directly moved from the legacy renderer for now:
void OONMainDisplay_sfml::render_scene()
// Should be idempotent -- doesn't matter normally, but testing could reveal bugs if it isn't!
{
	// Shape indexes must be the same as the corresponding entity indexes!
	for (size_t i = 0; i < shapes_to_change.size(); ++i) {
		auto& body = app().world().bodies[i];

		//!!Sigh, this will break as soon as not just circles would be there...
		auto shape = dynamic_pointer_cast<sf::Shape>(shapes_to_change[i]);

		// Set color (!!AND CURRENTLY: ALSO TEXTURE!!) to the avatar bg. for those that have an avatar!
		if (i != app().player_entity_ndx()) { //!! Currently only, also: "the", player has avatar...
			shape->setFillColor(sf::Color((body->color << 8) | p_alpha));
		} else {
			shape->setFillColor(sf::Color(avatar(oon_app().focused_entity_ndx).tint_RGBA));
			shape->setTexture(&(          avatar(oon_app().focused_entity_ndx).image));
		}

		auto& trshape = dynamic_cast<sf::Transformable&>(*shape);

		//!! The size and coords. of the screen view pane (UI viewport) are NOT directly
		//!! related to the camera view, but would obviously be best if they were identical!...

	// a)
		auto vpos = app().main_view().camera().world_to_view_coord(body->p); //!!?? No longer needed: ... - V2f(body->r, -body->r)); //!! Rely on the objects' own origin offset!
			                                                             //!! Mind the inverted camera & model y, too!
	// b)
	//	Szim::View::OrthoZoomCamera& oon_camera = (Szim::View::OrthoZoomCamera&) game.main_view().camera();
	//	auto vpos = oon_camera.world_to_view_coord(body->p); //!!?? No longer needed: ... - Math::Vector2f(body->r, -body->r)); //!! Rely on the objects' own origin offset instead!
	//	                                                     //!! Mind the inverted camera & model y, too!
		//!! Which they currently are NOT... The vertical axis (y) of the camera view is
		//!! a) inverted wrt. SFML (draw) coords., b) its origin is the center of the camera view.
		//!! -> #221, #445
		trshape.setPosition(sf::Vector2f{ vpos.x + float(app().main_window_width()/2),
			                         -vpos.y + float(app().main_window_height()/2)}); //!! "Standardize" on the view's centered origin instead!

//cerr << "render(): shape.setPos -> x = " << oon_camera.cfg.width /2 + (body->p.x) * oon_camera.scale() + oon_camera.offset.x
//			       << ", y = " << oon_camera.cfg.height/2 + (body->p.y) * oon_camera.scale() + oon_camera.offset.y <<'\n';
	}
}


void OONMainDisplay_sfml::draw() // override
{
	render_scene(); //!!?? Is this worth keeping separated from draw()?
	                //!!?? Probably, for occlusion etc. not relevant for any non-model frippery!

	// Grid lines...
	static sf::Color hair_color{0x44444488};
	if (const auto& cam = oon_camera(); cam.cfg.gridlines) {
		float min_x = 0, max_x = float(app().main_window_width());
		float min_y = 0, max_y = float(app().main_window_height());
		auto [vx, vy] = cam.grid_offset();//!!?? + Math::Vector2f{max_x/2, max_y/2};
		vx += max_x/2;
		vy = max_y/2 - vy;

		sf::Vertex vhair[] = {{{vx, min_y}, hair_color}, {{vx, max_y}, hair_color}};
		sf::Vertex hhair[] = {{{min_x, vy}, hair_color}, {{max_x, vy}, hair_color}};

		SFML_WINDOW(app()).draw(vhair, 2, sf::PrimitiveType::Lines);
		SFML_WINDOW(app()).draw(hhair, 2, sf::PrimitiveType::Lines);

/* Just do double-check the default SFML draw coords.:
sf::Vertex vcenterline[] = {{{max_x/2, min_y}, sf::Color::Black}, {{max_x/2, max_y}, sf::Color(0x88888844)}}; //!!?? WTF: no
sf::Vertex hcenterline[] = {{{min_x, max_y/2}, sf::Color::Black}, {{max_x, max_y/2}, sf::Color(0x88888844)}}; //!!?? alpha?! :-o
SFML_WINDOW(game).draw(vcenterline, 2, sf::PrimitiveType::Lines);
SFML_WINDOW(game).draw(hcenterline, 2, sf::PrimitiveType::Lines);
*/
	}

	//!!?? render(some target or context or options?) and is it worth separating from draw()?
	// Draw the world/scene...
	for (const auto& entity : shapes_to_draw) {
		SFML_WINDOW(app()).draw(*entity);
	}


	// Player halo...
	// Only if the real size is too small...
	const auto player_ndx = app().player_entity_ndx();
	const auto& player_body = app().player_entity();

	//!! May not remain a circle forever:
	auto& player_shape = (sf::CircleShape&) *(shapes_to_change[player_ndx]);

	auto rb = player_body.r * oon_camera().scale();
//	auto rb = ((sf::CircleShape&)player_shape).getRadius();
	static float A = 1.f; // pixel
	static float f = 2.0f; // Hz
	float phase = app().session_time() * f * 2*3.141f;
	float y = sin(phase);
	if (rb < 16) {
		float r = 20 + y * A/2;
		//r -= sin(phase) * 2/2; // Compensate when only pulsating the outline thickness.
		auto halo = sf::CircleShape(r);
		halo.setOutlineThickness(2); //(y * 2);
		halo.setOutlineColor(sf::Color(unsigned((app().entity(player_ndx).color << 8)
		                                        | 0x60 + unsigned(y * 20))));
		halo.setFillColor(sf::Color(0)); // sf::Color(0x66663333)
		halo.setOrigin({r, r});
		halo.setPosition(player_shape.getPosition());

		SFML_WINDOW(app()).draw(halo);
	}

	// Idle-rotate the player avatar...
	static float idle_rot_threshold = 2;
	static float idle_rot_rate_inv = 5;
	if (app().player_idle_time() > idle_rot_threshold)
	{
		player_shape.setRotation(
			sf::radians((app().player_idle_time() - idle_rot_threshold) / idle_rot_rate_inv)
		);
	} else {
		player_shape.setRotation(sf::radians(0));
	}

	//!!MOVE THIS TO THE UI:
	if (app().paused()) {
		draw_banner("PAUSED");
	}
}


//----------------------------------------------------------------------------
//!!MOVE TO UI::Widget::Notice!
void OONMainDisplay_sfml::draw_banner(const char* text) // override
{
	auto& game = app();

	if (!sfw::Theme::loadFont(game.cfg.asset_dir + game.cfg.default_font_file)) {
		//! SFML does print errors to the console.
		return;
	}

	auto TXT_WIDTH = 300u;
	auto TXT_HEIGHT = 80u;
	sfw::gfx::Text banner(text, TXT_HEIGHT); //!! Not UTF-8! :-/
	banner.setPosition({
		(float)SFML_WINDOW(game).getSize().x/2 - TXT_WIDTH/2,
		(float)SFML_WINDOW(game).getSize().y/2 - TXT_HEIGHT/2 - 16 //!!fuckup offset
	});
	banner.setStyle(sf::Text::Bold | sf::Text::Bold);
	banner.setFillColor(sf::Color(sf::Color(0xc0b0a08f))); //!!... Sigh... Get that color from somewhere! :)

	SFML_WINDOW(game).draw(banner);
}

} // namespace OON
