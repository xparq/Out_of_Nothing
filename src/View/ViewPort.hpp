#ifndef _DP2M97FG6MF98D59NH7_
#define _DP2M97FG6MF98D59NH7_

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
	static constexpr auto CFG_DEFAULT_ZOOM = 0.0000005f; //! This one also depends very much on the physics!

	Math::Vector2f world_to_view_coord(Math::Vector2f p) const { return p * zoom + offset; }
	Math::Vector2f world_to_view_coord(float x, float y) const { return { x * zoom + offset.x, y * zoom + offset.y }; }

	float zoom = CFG_DEFAULT_ZOOM;
	Math::Vector2f offset = {0, 0}; // in World-coordinates
}; // class ViewPort

} // namespace View

#endif // _DP2M97FG6MF98D59NH7_
