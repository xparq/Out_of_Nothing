#ifndef _MM49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_
#define _MM49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_

//!! Math is both not even needed for the metamodel itself (yet?), and also
//!! may not ONLY needed for modeling, so...
//!!#include "Szim/Math.hpp"
//!!#include "Szim/Math/Vector2.hpp"

#include <cstdint> // size_t; but use a type from the WorldConfig instead!

//----------------------------------------------------------------------------
namespace Szim {
namespace Model {

	using TimeSpan = double;

	//!! Make both of these to be unit32fast
	using EntityID = size_t; // See Entity.hpp for the definition of Entity::NONE (as ~0u)!
	using PlayerID = unsigned; //!! Should be "warninglessly compatible" with EntityID though!... ;)

	//!! Fckn' temp. hacks to overcome a crippling C++ namespacing/scoping limitation (see Entity::NONE)
	constexpr static EntityID NO_ENTITY = ~0u;
	constexpr static float    UNLIMITED = -1; //! Not enum to avoid the atrocity of `Entity::Enumname::Unlimited`

	//!! App-specific logic! But temporarily defined here (until it gets its own place finally):
	enum Event { None, Interacting, Collided, Terminated };

} // namespace Model
} // namespace Szim


#endif // _MM49N7856B98F4GHJN3MH58YU53MFYNIURTBY7NO568_

