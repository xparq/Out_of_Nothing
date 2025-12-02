#include "Szim/Model/World.hpp"
#include "Szim/diag/Log.hpp"


namespace Szim {
namespace Model::Core {

void World::update(TimeSpan dt)
// The updates are deterministic currently (but random effects may be added later).
{
//!!?? LOGE << app.world().props.repulsion_stiffness;

	//!! pre_update_hook(); // Per-tick model housekeeping before updating
	update_intrinsic(dt); // Advance entity states independently (O(n), can be parallel)
	update_pairwise(dt);  // Let them talk... (O(n²), probably (inherently?) sequential!)
	update_global(dt);    // Apply global effects (O(n), can be parallel)
	//!! post_update_hook(); // // Housekeeping after the update — i.e. before drawing!
}

} // namespace Model::Core
} // namespace Szim
