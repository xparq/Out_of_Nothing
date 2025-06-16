#ifndef _F49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_
#define _F49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_

//#include "Metamodel/cfg.hpp"

//!! That "Model/"" is a legacy kludge until Math is migrated to the Engine(/Metamodel):
#include "Model/Math.hpp"
#include "Model/Math/Vector2.hpp"

namespace Model {

/*!!
// Abstract world
struct World
{
};
!!*/


//!! Temporarily defined here (until it grows to need its own place):
enum Event { None, Interacting, Collided, Terminated };


//!! JUNK:
constexpr static float Unlimited = -1; //! Not enum to avoid the atrocity of `Entity::Enumname::Unlimited`

} // namespace Model

#endif // _F49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_

