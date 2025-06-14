#ifndef _O9090N7B8C12345607N772V8456N720C45Y69780W5B76980YH_
#define _O9090N7B8C12345607N772V8456N720C45Y69780W5B76980YH_

#include <cstddef> // size_t, but #592!

namespace Szim {

class Avatar;
class VirtualController;

class Player
{
public:
	Player(size_t entity_ndx, Avatar& avatar, VirtualController& controls);
	virtual ~Player();

//----------------------------------------------------------------------------
// Data
//
	//--------------------------------------------------------------------
	// Config
	size_t      entity_ndx = ~0u; // Associated (root) model entity
	Avatar*     avatar = nullptr;
	VirtualController* controls = nullptr;

	//--------------------------------------------------------------------
	// State
	float last_action_time = 0;
};

} // namespace Szim

#endif // _O9090N7B8C12345607N772V8456N720C45Y69780W5B76980YH_