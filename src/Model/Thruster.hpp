#ifndef _3895UY9G8HJ6987VN9836457NF48567459B8GV7UUBTNBRGJLKH_
#define _3895UY9G8HJ6987VN9836457NF48567459B8GV7UUBTNBRGJLKH_

#include "Emitter.hpp"

namespace Model {

struct Thruster //!! : public Entity //!! Not only not ready for that, but it's even the other way around: Entity #includes Thruster currently!... :-o
{
	//!!Thrusters should be vectorized, relative to the body orientation,
	//!!which is currently fixed to be identical to the world coordinate system...
	float _thrust_level = 0;
	float thrust_level(float new_thrust_level)
	{
		auto prev_thrust_level = _thrust_level;
		_thrust_level = new_thrust_level;
		return prev_thrust_level;
	}
	float thrust_level() const { return _thrust_level; }
};

} // namespace Model

#endif // _3895UY9G8HJ6987VN9836457NF48567459B8GV7UUBTNBRGJLKH_
