#ifndef _KJHWERHKWHQNCEURTHNY48_
#define _KJHWERHKWHQNCEURTHNY48_

#include <cstdint>
//!!#include <functional> // function<> (e.g. for timers)

namespace Szim {

// Update cycle counters
// (e.g. for the number of update cycles, all throughout a long-running session)
// By default they use the native machine word size.
// Note: 1 calendar year = 3,784,320,000 cycles at 120 fps, so even 32 bits are plenty.
// For longer runs (e.g. persistent-world servers) or higher resolution, compile for 64-bit!
template<int> struct CycleCount_;
template<> struct CycleCount_<4> { typedef uint32_t type; };
template<> struct CycleCount_<8> { typedef uint64_t type; };
typedef CycleCount_<sizeof(void*)>::type CycleCount;

typedef float Seconds;

	struct Time
	{
	// Controls
		bool  paused = false;
		bool  reversed = false;
		float scale = 1.0f; // > 0 (Decoupled from time reversal, for more flexible controls.)

	// State
		Seconds session_time = 0; // life-time of this Time instance
		Seconds dt_last; // ...and here's a Δ for copy-pasting :)
//!!		Seconds dt_min;
//!!		Seconds dt_max;
//!!		Seconds dt_avg100;
		Seconds last_frame_delay; // In some modes it's not tied to Δt at all!
		unsigned fps_limit = 0;   // 0: no limit; see: fps_throttling(), cfg.fps_limit

//!!		seconds Δt_since_last_query(std::function<seconds(SimApp&)>) timer_snapshot_callback);

		//!! Also keep a limited time series irrespective of the running avgs,
		//!! so that it can be examined retrospectively for diagnistics.
		//!! (Note: feeding live perf. graphs dont' need it, they keep their own data.)

		/* Δt modes:// Not really time modes tho, but more generic engine/run modes...

			last-frame: dt is an estimated prediction based on a rolling avg.
			
			fixed / sealed: this can do proper backstepping, so that
				the even the main cycle cound could probably be decreased reliably
				(well, only if any uncontrolled, random inputs (like user actiona
				are tightly shut out!)

			fixed / real-time
		*/
	};

} // namespace Szim
#endif // _KJHWERHKWHQNCEURTHNY48_
