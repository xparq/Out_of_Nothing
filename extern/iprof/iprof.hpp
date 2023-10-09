// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#pragma once

#ifndef IPROF_DISABLE

#if (defined(_MSC_VER) && (_MSC_VER < 1916)) || defined(EMSCRIPTEN) || defined(CC_TARGET_OS_IPHONE) || defined(__ANDROID__)
# define IPROF_DISABLE_MULTITHREAD
#endif

#ifndef IPROF_DISABLE_VECTOR_OPT
# include "lossyvector.hpp" // vector replacement optimized for use in std::map
#endif
#include <vector> // needed anyway for other things
#include <map>
#ifndef IPROF_DISABLE_MULTITHREAD
# include <mutex>
#endif
#include <ostream>

#ifdef IPROF_DISABLE_MULTITHREAD
# define iprof_thread_local
#else
# define iprof_thread_local thread_local
#endif

// windows.h and <chrono> can't survive including from within a namespace, so
// we must do this prior to including (actually: embedding) the clock stuff:
#include <chrono>
#ifdef _MSC_VER
# include <Windows.h>
#endif

namespace iProf
{
	#include "hirestime.hpp" // See comment above...

#ifndef IPROF_DISABLE_VECTOR_OPT
	typedef LossyVector<const char*, 15> TagList;
		// With 15, sizeof == 64 (15*4 + 2 + 2) for 32-bit systems,
		// and should be (aligned to) 128 for 64-bit systems.
#else
	typedef std::vector<const char*> TagList;
#endif

struct Measurement
{
	TagList scopePath;
	HiResTime::time_point tStart, tStop;

	Measurement(const TagList& path) :
		scopePath(path),
		tStart(HiResTime::now()),
		tStop(tStart - HiResTime::duration(1)) // invalid interval means running()
	{}
	constexpr bool running() { return tStart > tStop; }
};

struct Totals
{
	HiResTime::duration tTotal = HiResTime::duration::zero();
	size_t nVisits = 0;

	Totals& operator+=(const Totals& a)
	{
		tTotal += a.tTotal;
		nVisits += a.nVisits;
		return *this;
	}
	Totals& operator-=(const Totals& a)
	{
		tTotal -= a.tTotal;
		nVisits -= a.nVisits;
		return *this;
	}
};

typedef std::map<TagList, Totals> Stats;   // LossyVector lacks hashing required by unordered_map
typedef std::vector<Measurement> Measurements;

extern iprof_thread_local Stats stats;
extern iprof_thread_local Measurements measurements; //!!?? Why was this one not thread-local before?
extern iprof_thread_local TagList currentScopePath;
#ifndef IPROF_DISABLE_MULTITHREAD
extern Stats allThreadStats;
extern std::mutex allThreadStatLock;
#endif

void accumulateLatestMeasurements();
void addThisThreadEntriesToAllThreadStats();
void clear();


inline const Stats& getStats()
{
// IPROF_SYNC
	iProf::accumulateLatestMeasurements();
// IPROF_SYNC_THREAD
//!!	iProf::addThisThreadEntriesToAllThreadStats(); //!! NO, this can't help at the end, if we're in another thread already!

	return stats;
}

inline void start(const char* tag)
{
	currentScopePath.push_back(tag);
	measurements.emplace_back(Measurement{currentScopePath});
//	measurements.emplace_back(currentScopePath); // This seemed consistently slower with CLANG! :-o
}
inline void stop()
{
	// Find the pending measurement of the current scope
	auto depth = currentScopePath.size();
	auto m = measurements.end();
	do --m; while (m->scopePath.size() != depth); // Skip possible newer ones in inner scopes

	m->tStop = HiResTime::now();
	currentScopePath.pop_back();
}

struct ScopedMeasure
{
	ScopedMeasure(const char* tag) { start(tag); }
	~ScopedMeasure() { stop(); }
};


// To avoid conflicts in the IPROF_ macros between HiResTime and
// iProf::HiResTime, if the user has also included hirestime.hpp:
namespace HRTime = HiResTime;
} // namespace iProf

std::ostream& operator<<(std::ostream& os, const iProf::Stats& stats);

#ifndef __FUNCTION_NAME__
# ifdef _MSC_VER
#  define __FUNCTION_NAME__ __FUNCTION__
# else
#  define __FUNCTION_NAME__ __func__
# endif
#endif

#define _IPROF_CONCAT_2_(a, b) a##b
#define _IPROF_CONCAT_(a, b) _IPROF_CONCAT_2_(a, b)

#define IPROF(n) iProf::ScopedMeasure _IPROF_CONCAT_(iProf__,__COUNTER__)(n)
#define IPROF_FUNC iProf::ScopedMeasure _IPROF_CONCAT_(iProf__,__COUNTER__)(__FUNCTION_NAME__)
#define IPROF_SYNC iProf::accumulateLatestMeasurements()
#define IPROF_SYNC_THREAD { IPROF_SYNC; iProf::addThisThreadEntriesToAllThreadStats(); }
/*!!
#ifdef ...IPROF_DISABLE_MULTITHREAD?? -> #9
# define IPROF_STATS        iProf::stats
#else
# define IPROF_STATS        iProf::allThreadStats
//!! Also, this does not actually exist in the end, IPROF_SYNC_THREAD kinda kills it, AFAICU:
//!!# define IPROF_THREAD_STATS iProf::stats
#endif
!!*/
#define IPROF_STATS        iProf::getStats() // See #17: not just `stats`, to support implicit syncing...
                                             //!! AFAICU, IPROF_STATS is invalid in multi-threaded runs: IPROF_SYNC_THREAD kills it!
#define IPROF_ALL_THREAD_STATS iProf::allThreadStats
#define IPROF_NOW          iProf::HRTime::now()
#define IPROF_MICROSEC(x)  iProf::HRTime::MICROSEC(x)
#define IPROF_MILLISEC(x)  iProf::HRTime::MILLISEC(x)
#define IPROF_SEC(x)       iProf::HRTime::SEC(x)

#else // IPROF_DISABLE
# define IPROF(n)
# define IPROF_FUNC
# define IPROF_NOW          (0)
# define IPROF_SYNC         void(0)
# define IPROF_SYNC_THREAD  void(0)
# define IPROF_STATS        (0)
# define IPROF_ALL_THREAD_STATS (0)
# define IPROF_MICROSEC(x)  (0)
# define IPROF_MILLISEC(x)  (x) // Still process x, to avoid subtle nightmare bugs!
# define IPROF_SEC(x)       (x) // Still process x, to avoid subtle nightmare bugs!
#endif // IPROF_DISABLE
