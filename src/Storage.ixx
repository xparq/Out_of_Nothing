module;

#include "Model/World.hpp"
#include <cstdint>

export module Storage;

namespace MEMDB {
	export unsigned constexpr MAX_WORLD_SNAPSHOTS = 10;
	export Model::World world_snapshots[MAX_WORLD_SNAPSHOTS];

	export uint16_t saved_slots = 0x0000; //! bitfield
};
