#ifndef _FNVNB807K8CD893IO3OIEWOIXMO9822894567B_
#define _FNVNB807K8CD893IO3OIEWOIXMO9822894567B_

#include "Engine/Metamodel.hpp"
//!! Legacy kludge until the relevant generic Physics parts (Pos) are migrated to the Engine(/Metamodel):
#include "Model/Physics.hpp"

namespace Szim::View {

// Camera image (view) coordinates (right-handed, like OpenGL's):
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
	using ViewPos  = Math::V2f; // Screen space (so always 2D)! The rebased (possibly 3D) "view space" for the view frustum (projection) IS NOT YET MODELLED AT ALL!
	using WorldPos = Model::Phys::Pos2;
	//!! TBD... (Probably translated, and/or zoomed ViewPos; review its use cases!):
	//!! This may not be the best here, with all those awkward conversions:
	//!! perhaps elsewhere in the rendering chain, closer to rasterizing,
	//!! where things tend to be integer already anyway!
	using V2f = Math::V2f;

	virtual ViewPos  world_to_view_coord(WorldPos wpos) const = 0;
	virtual WorldPos view_to_world_coord(ViewPos vpos) const = 0;

//!! Don't try to add this yet again! ;) The camera is always "centered"; it only makes sense to (re)orient it to something!
//!! And the point {0,0} is not a special "something" at all.
//!!	virtual void center() = 0;

	virtual float scale() const { return 1; }

	virtual ~Camera() = default;
}; // class Camera

} // namespace Szim::View

#endif // _FNVNB807K8CD893IO3OIEWOIXMO9822894567B_
