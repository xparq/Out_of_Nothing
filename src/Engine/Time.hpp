#ifndef _KJHWERHKWHQNCEURTHNY48_
#define _KJHWERHKWHQNCEURTHNY48_

#include "sz/stat/collect.hh" // generic min/max/avg perf. counter

#include <cstdint>
//!!#include <functional> // function<> (e.g. for timers)

namespace Szim::Time {

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

	struct Control
	{
	// Controls
		bool  paused = false;
		bool  reversed = false;
		float scale = 1.0f; // > 0, so it's decoupled from time reversal (for more flexible controls)

	// State
		Seconds last_frame_delay; // In some modes it's not tied to the model Δt at all!
		Seconds last_model_Δt;
		Seconds real_session_time = 0; // (real-world) life-time of this Time instance

		//!! These should be kept in the model world!
		//!! The app can come and go, while a persistent model is still up,
		//!! and conversely: models can be loaded/reset, outside the view on
		//!! this feeble class, at least as-is: it's not really in control yet!...
		//!!Seconds total_model_time; // Age of the virtual universe (neg. time-stepping decreases it!)
		sz::stats::last_total_min_max<Seconds> model_Δt_stats;

		//!! Also keep a limited time series irrespective of the running avgs,
		//!! so that it can be examined retrospectively for diagnistics.
		//!! (Note: feeding live perf. graphs don't need it, they keep their own data.)

		/* Δt modes:// Not really time modes tho, but more generic engine/run modes...

			last-frame: dt is an estimated prediction based on a rolling avg.
			
			fixed / sealed: this can do proper backstepping, so that
				the even the main cycle cound could probably be decreased reliably
				(well, only if any uncontrolled, random inputs (like user actiona
				are tightly shut out!)

			fixed / real-time
		*/
	};

} // namespace Szim::Time
#endif // _KJHWERHKWHQNCEURTHNY48_
