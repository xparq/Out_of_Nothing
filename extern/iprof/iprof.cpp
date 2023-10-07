// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

//#define IPROF_DISABLE_MULTITHREAD // Testing #12
//#define IPROF_DISABLE_VECTOR_OPT // Testing #13
#include "iprof.hpp"

namespace iProf
{
iprof_thread_local Stats stats;
iprof_thread_local Measurements measurements;
iprof_thread_local TagList currentScopePath;

#ifndef IPROF_DISABLE_MULTITHREAD
std::mutex allThreadStatLock;
Stats allThreadStats;
#endif

void accumulateLatestMeasurements()
{
	std::vector<Measurement> unfinished;
	if (measurements.size() > 4)
		unfinished.reserve(measurements.size() >> 1);
	for (auto measurement : measurements)
	{
		if (measurement.running())
		{
			unfinished.emplace_back(measurement);
			continue;
		}
		Totals& stat = stats[measurement.scopePath];
		++stat.nVisits;
		stat.tTotal += measurement.tStop - measurement.tStart;
	}
	std::swap(measurements, unfinished);
}

#ifndef IPROF_DISABLE_MULTITHREAD
void addThisThreadEntriesToAllThreadStats() //!! Should be idempotent!
{
	iprof_thread_local Stats lastStats;
	std::lock_guard<std::mutex> bouncer(allThreadStatLock);
	for (const auto& [path, stat] : lastStats)
		allThreadStats[path] -= stat;
	for (const auto& [path, stat] : stats)
		allThreadStats[path] += stat;
	lastStats = stats;
}
#endif

void clear()
{
	measurements.clear();
	for (auto& [path, stat] : stats)
	{
		stats[path] = Totals{HiResTime::duration(0), 0};
#ifndef IPROF_DISABLE_MULTITHREAD
		allThreadStats[path] = Totals{HiResTime::duration(0), 0};
#endif
	}
}
} // namespace iProf

std::ostream& operator<<(std::ostream& os, const iProf::Stats& stats)
{
	for (const auto& [path, data] : stats)
	{
		for (auto& tag : path)
			os << tag << (&tag != &path.back() ? "/" : "");
		if (path.capacity() < path.size())
			os << "/...(" << path.size() - path.capacity() << ")";
		os << ": " << iProf::HiResTime::MICROSEC(data.tTotal) / float(data.nVisits)
			<< " μs (" << iProf::HiResTime::MICROSEC(data.tTotal)
			<< " μs / " << data.nVisits << ")\n";
	}
	return os;
}
