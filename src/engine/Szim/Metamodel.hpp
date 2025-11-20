#ifndef _F49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_
#define _F49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_

//#include "Metamodel/cfg.hpp"

//!! That "Model/"" is a legacy kludge until Math is migrated to the Engine(/Metamodel):
#include "Model/Math.hpp"
#include "Model/Math/Vector2.hpp"

//----------------------------------------------------------------------------
namespace Szim {

/*!!
// Abstract world
struct World
{
};
!!*/

} // namespace Szim


//----------------------------------------------------------------------------
namespace Model {

	using EntityID = size_t; // See Entity.hpp for the definition of Entity::NONE (as ~0u)!
	using PlayerID = unsigned; //!! But this should be "warninglessly" compatible with EntityID though!...

	//!! Fckn' temp. hacks to overcome a crippling C++ namespacing/scoping limitation (see Entity::NONE)
	constexpr static EntityID NO_ENTITY = ~0u;
	constexpr static float    UNLIMITED = -1; //! Not enum to avoid the atrocity of `Entity::Enumname::Unlimited`

	//!! App-specific logic! But temporarily defined here (until it gets its own place finally):
	enum Event { None, Interacting, Collided, Terminated };
}

#endif // _F49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_

