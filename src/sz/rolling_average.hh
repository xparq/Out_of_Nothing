#ifndef _SZ_ROLLING_AVG_HH_
#define _SZ_ROLLING_AVG_HH_

namespace sz {

template<int MAX_SAMPLES, typename TFloat = float>
struct RollingAverage
{
	TFloat samples[MAX_SAMPLES] = {};
	TFloat sum = 0;
	short tail = 0;

	void update(TFloat new_sample) {
		sum += new_sample - samples[tail];
		samples[tail++] = new_sample;
		tail %= MAX_SAMPLES;
	}

	// Queries (Note: initial results are incorrect during ramp-up!)
	operator TFloat() const { return sum / MAX_SAMPLES; }
	operator int()    const { return int(sum / MAX_SAMPLES); }
};

#ifndef TFloat
#define TFloat_tmp
#define TFloat float // Or can we have template arg deduction from other args?
#endif
template<TFloat smoothing_factor = (TFloat)0.99f, TFloat initial = (TFloat)0.f>
struct SmoothRollingAverage
{
	TFloat smoothed = initial; // exp. smoothed average

	void update(TFloat new_sample) {
		smoothed = smoothing_factor * smoothed + (1 - smoothing_factor) * new_sample;
		// Not doing this; if the initial bias is a problem, client code can preset it instead!
		//	smoothed = smoothed ? // Don't let the initial 0 skew it!
		//		     smoothing_factor * smoothed + (1 - smoothing_factor) * new_sample
		//		   : new_sample;
	}

	// Queries
	operator TFloat() const { return smoothed; }
	operator int()    const { return int(smoothed); }
};
#ifdef TFloat_tmp
#undef TFloat_tmp
#undef TFloat
#endif

}; // namespace
#endif _SZ_ROLLING_AVG_HH_
