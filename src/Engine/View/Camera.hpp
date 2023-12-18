#ifndef _FNVNB807K8CD893IO3OIEWOIXMO9822894567B_
#define _FNVNB807K8CD893IO3OIEWOIXMO9822894567B_

#include "Model/Math/Vector2.hpp"

namespace Szim::View {

// Camera image (view) coordinates (right-handed, like OpenGL):
//
//       +y
//        |
//  -x -- 0 -- +x
//        |
//       -y
//
// Origin: center of the view

class Camera
{
public:
	virtual Math::Vector2f world_to_view_coord(Math::Vector2f wpos) const = 0;
	virtual Math::Vector2f view_to_world_coord(Math::Vector2f vpos) const = 0;

	virtual float scale() const { return 1; }

	virtual ~Camera() = default;
}; // class Camera

} // namespace Szim::View

#endif // _FNVNB807K8CD893IO3OIEWOIXMO9822894567B_
