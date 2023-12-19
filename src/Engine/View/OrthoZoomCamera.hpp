#ifndef _DP2M97FG6MF98D59NH70873456874G3076589_
#define _DP2M97FG6MF98D59NH70873456874G3076589_

#include "Camera.hpp"

#include "Model/Math/Vector2.hpp"

namespace Szim::View {

// Camera View coordinate system (right-handed, like OpenGL):
//
//       +y
//        |
//  -x -- 0 -- +x
//        |
//       -y
//
// Origin: center of the view

struct OrthoZoomCamera : Camera
{
	struct Config
	{
		// Why float dimensions? -> #221: Camera concept/definitions...
		float width  = 1024;
		float height = 768;
		float base_scale = 1; // Depends on the physics, so pretty much always set it!
		bool  gridlines = true; //!! -> false, as soon as it involves complexity beyond a simple direct query!
	};

	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------

//	OrthoZoomCamera();
	OrthoZoomCamera(Config cfg);
	void reset(const Config* recfg = nullptr); // Resets things to the last cfg if null.
	void reset(Config&& recfg);
	void resize(float width, float height);

	// -------------------------------------------------------------------
	// Queries...
	// -------------------------------------------------------------------

	// Simple orthographic projection, plus scaling:
	Math::Vector2f world_to_view_coord(Math::Vector2f wp) const override { return wp * _scale - offset; } 
//	Math::Vector2f world_to_view_coord(float wx, float wy) const { return world_to_view_coord({wx, wy}); }

	Math::Vector2f view_to_world_coord(Math::Vector2f vp) const override { return (vp + offset)/_scale; }
//	Math::Vector2f view_to_world_coord(float vx, float vy) const { return view_to_world_coord({vx, vy}); }

	float scale() const override { return _scale; }

	bool visible(Math::Vector2f world_pos) const {
		auto vpos = world_to_view_coord(world_pos);
		return visible_x(vpos.x) && visible_y(vpos.y);
	}
	// These are to check view(!) positions (per dim.):
	bool visible_x(float view_pos_x) const { return view_pos_x >= _edge_x_min && view_pos_x < _edge_x_max; }
	bool visible_y(float view_pos_y) const { return view_pos_y >= _edge_y_min && view_pos_y < _edge_y_max; }

	//!! This may not be best here, with all those awkward conversions:
	//!! perhaps elsewhere in the rendering chain, closer to rasterizing,
	//!! where things tend to be integer already anyway!
	Math::Vector2f grid_offset() const;

	//!! This does NOT belong here! -> #221
	Math::Vector2f screen_to_view_coord(int x, int y) const; //!! { return {(float)x + _edge_x_min, _edge_y_max - (float)y}; }
		//!! Used currently to map e.g. mouse positions back to the world.
		//!! (See also entity_at_wiewpos(...)!)
		//!! The UI certainly needs it, but it should do OrthoZoomCamera - ScreenView conversions
		//!! itself, and then ask the camera for Camera - World ("frustum line") conversion,
		//!! and then ask the model to find the object hit by that line!
		//!! Also, Renderer::render() does this conversion, too
		//!! Till then...
		//!! This would also require properly defining screen coords first (via the ctor)! :-o
		//!! Now it just assumes direct mapping between the screen (window) and the camera view!

	// -------------------------------------------------------------------
	// Operations...
	// -------------------------------------------------------------------

	// Movement
	void pan(Math::Vector2f delta) { pan_x(delta.x); pan_y(delta.y); }
	void pan_x(float delta)        { offset.x += delta; }
	void pan_y(float delta)        { offset.y += delta; }
	void center_to(Math::Vector2f world_pos) { offset = world_pos * _scale; }

	// Moving the focus center only
	void move_focus(Math::Vector2f delta) { focus_offset += delta; }
	void move_focus_x(float dx) { focus_offset.x += dx; }
	void move_focus_y(float dy) { focus_offset.y += dy; }
	void focus_to(Math::Vector2f view_pos) { focus_offset = view_pos; }

	// Zoom
	void zoom(float change_factor); // Zoom change ratio (zoom in if > 1)
	void zoom_in  (float step) { zoom(1.f + step); }
	void zoom_out (float step) { zoom(1.f / (1.f + step)); }

	//!!?? This "intelligence" may be better elsewhere?
	//!!?? This + all the visibility checks (mostly only used by this...) are the
	//!!?? only reason view(finder) dimensions + resizing is needed here at all! :-o
	//!!?? AFAIK it's also separated (as the clip space coordinate system) in OpenGL, too!
	bool confine(Math::Vector2f world_pos, float viewpos_margin = 0, float throwback = 2); // returns true if the player was out of view


	// --- "API Data" ----------------------------------------------------
	Config cfg;

	float _scale = 1;
	Math::Vector2f offset = {0, 0}; // Displacement of the view relative to the initial implicit origin, in View (screen) coordinates
	Math::Vector2f focus_offset = {0, 0}; // Pos. of a focus point in the image rect (in Camera View coord.)
	                                      // Used as the zoom origin for now. (Usually set to the player's
	                                      // on-screen pos, or some other interesting subject...)

	// --- Internals -----------------------------------------------------
protected:
	// Frequently needed convenience params. (i.e. cached values calculated from the cfg.):
	float _edge_x_min, _edge_x_max;
	float _edge_y_min, _edge_y_max;

}; // class OrthoZoomCamera

} // namespace Szim::View

#endif // _DP2M97FG6MF98D59NH70873456874G3076589_
