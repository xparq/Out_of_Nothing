#ifndef _MD09245876B87GV4456CN6B7_
#define _MD09245876B87GV4456CN6B7_

#include "Szim/Backend/Clock.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

namespace Szim::Time {

struct SFML_Clock : Clock
{
	sf::Clock _sfml_clock;
	operator sf::Clock&() { return _sfml_clock; }

//	virtual void reset();
//	virtual void start();
//	virtual void stop();
	virtual void restart() override { _sfml_clock.restart(); }
	virtual Seconds get()  override { return _sfml_clock.getElapsedTime().asSeconds(); }
};


/*!!DIGEST:
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
#endif // _MD09245876B87GV4456CN6B7_
