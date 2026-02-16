#ifndef _3895UY9G8HJ6987VN9836457NF48567459B8GV7UUBTNBRGJLKH_
#define _3895UY9G8HJ6987VN9836457NF48567459B8GV7UUBTNBRGJLKH_

#include "Emitter.hpp"
#include "app/Model/Physics.hpp" // Phys::NumType

namespace OON::Model {

struct Thruster //!! : public Entity //!! Not only not ready for that, but it's even the other way around: Entity #includes Thruster currently!... :-o
//!! Thrusters should be vectorized (relative to body orientation,
//!! which is currently fixed: axis-aligned to world coord. system)...
{
	using NumType = float; // Not Phys::NumType! If that's 64-bit, it'd be terribly wasteful for entity data!
	                       // Even float might get downgraded to float16 in the future. (There are 4 thrusters
	                       // per entity now, so it wouldn't throw off the alignment either.)

	NumType _thrust_level = 0;

	NumType thrust_level(Phys::NumType new_thrust_level)
	{
		auto prev_thrust_level = _thrust_level;
		_thrust_level = static_cast<decltype(_thrust_level)>(new_thrust_level); // Silencing "possible loss of data" warnings...
		return prev_thrust_level;
	}
	NumType thrust_level() const { return _thrust_level; }
};

} // namespace OON::Model

#endif // _3895UY9G8HJ6987VN9836457NF48567459B8GV7UUBTNBRGJLKH_
