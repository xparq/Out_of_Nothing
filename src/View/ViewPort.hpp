#ifndef _DP2M97FG6MF98D59NH7_
#define _DP2M97FG6MF98D59NH7_

#include "Model/Math/Vector2.hpp"

namespace View {

// View-local coord. sys.:
//
//       +y
//        |
//  -x -- 0 -- +x
//        |
//       -y
//
// Origin: center of the screen (window, view pane...)


struct ViewPort
{
	struct Config
	{
		// Why float dimensions? -> #221: Camera concept/definitions...
		float width  = 1024;
		float height = 768;
		float base_scale = 1; // Depends on the physics, so pretty much always set it!
	};

	// -------------------------------------------------------------------
	ViewPort(Config cfg);
	void reset(const Config* recfg = nullptr); // Resets things to the last cfg if null.
	void reset(Config&& recfg);
	void resize(float width, float height);

	// -------------------------------------------------------------------
	Math::Vector2f world_to_view_coord(Math::Vector2f p) const { return p * scale - offset; }
	Math::Vector2f world_to_view_coord(float x, float y) const { return { x * scale - offset.x, y * scale - offset.y }; }

	Math::Vector2f view_to_world_coord(Math::Vector2f vp) const { return (vp + offset)/scale; }
	//!!Math::Vector2f view_to_world_coord(float x, float y) const { ... }

//!! This would require properly defining a screen coords first via the ctor! :-o
	Math::Vector2f screen_to_view_coord(int x, int y) const { return {(float)x + _edge_x_min, (float)y + _edge_y_min}; }

	void center_to(Math::Vector2f world_pos) {
		offset = world_pos * scale;
	}

	void pan(Math::Vector2f delta) { pan_x(delta.x); pan_y(delta.y); }
	void pan_x(float delta)        { offset.x += delta; }
	void pan_y(float delta)        { offset.y += delta; }

	void zoom(float change_factor); // Zoom change ratio (zoom in if > 1)
	void zoom_in  (float step) { zoom(1.f + step); }
	void zoom_out (float step) { zoom(1.f / (1.f + step)); }

	bool visible(Math::Vector2f world_pos) const {
		auto vpos = world_to_view_coord(world_pos);
		return visible_x(vpos.x) && visible_y(vpos.y);
	}
	// These are to check view(!) positions (per dim.):
	bool visible_x(float view_pos_x) const { return view_pos_x >= _edge_x_min && view_pos_x < _edge_x_max; }
	bool visible_y(float view_pos_y) const { return view_pos_y >= _edge_y_min && view_pos_y < _edge_y_max; }

	bool confine(Math::Vector2f world_pos, float viewpos_margin = 0, float throwback = 2); // returns true if the player was out of view


	// --- "API Data" ----------------------------------------------------
	Config cfg;

	float scale = 1;
	Math::Vector2f offset = {0, 0}; // Displacement of the view relative to the initial implicit origin, in View (screen) coordinates
	Math::Vector2f focus_offset = {0, 0}; // Pos. of a focus point in the view rect (in View coord.)
	                                      // Used as the zoom origin for now. (Usually set to the player's
	                                      // on-screen pos, or some other interesting subject...)

	// --- Internals -----------------------------------------------------
protected:
	// Frequently needed convenience params. (i.e. cached values calculated from the cfg.):
	float _edge_x_min, _edge_x_max;
	float _edge_y_min, _edge_y_max;

}; // class ViewPort

} // namespace View

#endif // _DP2M97FG6MF98D59NH7_
