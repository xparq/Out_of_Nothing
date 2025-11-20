#ifndef _94857627NC4586BV987C4H_
#define _94857627NC4586BV987C4H_

#include "Szim/Time.hpp" //!! This is off. Instead, Time.hpp should be split and both
                           //!! that and this should include an extracted low-level part!

namespace Szim::Time {

struct Clock
{
//	virtual void reset();
//	virtual void start();
//	virtual void stop();
	virtual void restart() = 0;
	virtual Seconds get() = 0;

	virtual ~Clock() = default;
};

/*!!
	class Stopwatch
	{
		enum Mode { Start, Hold, Stop, Reset } mode;
		using callback_t std::function<seconds()>
		callback_t capture_callback;
	public:
		Stopwatch(callback_t timer_snapshot_callback, Mode mode = Hold):
			capture_callback(timer_snapshot_callback)
		{
		}
		seconds stop(std::function<seconds(SimApp&)>) timer_snapshot_callback);
	};
!!*/

} // namespace Szim::Time
#endif // _94857627NC4586BV987C4H_
