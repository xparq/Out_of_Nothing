// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#pragma once

#include <chrono>
#ifdef _MSC_VER
# include <Windows.h>
#endif

namespace HiResTime
{
#ifndef _MSC_VER

	typedef std::chrono::high_resolution_clock Clock;

#else

	namespace
	{
		const auto g_Frequency = []() {
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			return frequency.QuadPart;
		}();
	}

	struct Clock
	{
		typedef long long                           rep;
		typedef std::nano                           period;
		typedef std::chrono::duration<rep, period>  duration;
		typedef std::chrono::time_point<Clock>      time_point;

		static const bool is_steady = true;

		static inline time_point now()
		{
			LARGE_INTEGER count;
			QueryPerformanceCounter(&count);
			return time_point(duration(count.QuadPart * static_cast<rep>(period::den) / g_Frequency));
		}
	};

#endif // _MSC_VER

	using duration = Clock::duration;
	using time_point = Clock::time_point;
	inline auto now() { return Clock::now(); }

	inline auto MICROSEC(const duration& x) { return std::chrono::duration_cast<std::chrono::microseconds>(x).count(); } //!! rename to microseconds()
	inline auto MILLISEC(const duration& x) { return std::chrono::duration_cast<std::chrono::milliseconds>(x).count(); } //!! rename to milliseconds()
	inline auto      SEC(const duration& x) { return std::chrono::duration_cast<std::chrono::seconds>(x).count(); }      //!! rename to seconds()

} // namespace HiResTime
