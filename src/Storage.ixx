//!! Just a dummy exercise & smoke test for the build procedure for now...

module;

//#include "Model/World.hpp"
#include <cstdint>

export module Storage;

/*
namespace MEMDB {
	export unsigned constexpr MAX_WORLD_SNAPSHOTS = 10;
	export Model::World world_snapshots[MAX_WORLD_SNAPSHOTS];

	export uint16_t saved_slots = 0x0000; //! bitfield
};
*/

/*!! GCC says: "sorry, unimplemented: private module fragment"
module: private;

//...private stuff not exported

//!!?? So how does it differ from stuff above this private fragment that are simply not `export`ed?!

!!*/
