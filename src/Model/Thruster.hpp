#ifndef __MODEL__THRUSTER__
#define __MODEL__THRUSTER__

struct Thruster
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

#endif // __MODEL__THRUSTER__