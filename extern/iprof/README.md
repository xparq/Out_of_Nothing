# iprof - a pretty simple and performant C++ profiling library

The iprof library let's you measure the performance of your C++ code in real time, with little overhead.

## Usage

First of all, include `iprof.hpp`.

### Measurements

In any function you want to measure (duration and number of runs), add the `IPROF_FUNC` macro:

```C++
#include "iprof.hpp"

// ...

void suspectedPerformanceCulprit()
{
   IPROF_FUNC;
   // ...
}
```

(Note: it will measure the time between `IPROF_FUNC` and returning from the function, so you can exclude prelude code if you wish.)

You can also gather stats for any other scope:

```C++
// ...
{
   IPROF("HeavyStuff");
   // ...
}
// ...
```

The statistics are gathered in `IPROF_STATS`.


### Reports

The results can be easily streamed out as text with the provided `<<` operator:

```C++
std::cerr << "The latest profiling stats:\n"
          << IPROF_STATS << '\n';
```

The `<<` operator formats the results in the following manner:

```text
SCOPE: AVG_TIME unit (TOTAL_TIME unit / TIMES_EXECUTED)
```

`unit` is always `μs` (microsecond), until support for others gets implemented...

Sample output:

```text
heavyCalc: 904026 μs (904026 μs / 1)
heavyCalc/bigWave: 148.25 μs (148250 μs / 1000)
heavyCalc/hugePower: 755.53 μs (755530 μs / 1000)
heavyCalc/hugePower/SecondPowerLoop: 32.733 μs (32733 μs / 1000)
heavyCalc/hugePower/BigWavePowerLoop: 445.48 μs (445480 μs / 1000)
heavyCalc/hugePower/FirstPowerLoop: 276.753 μs (276753 μs / 1000)
heavyCalc/hugePower/BigWavePowerLoop/bigWave: 148.288 μs (444863 μs / 3000)
```

### "Syncing" measurement data

To make iprof more performant (you don't want your profiler to perturb the
measurements by being slow), the individual measurements (often in
hot code) only record the results, without also automatically feeding them
to the overall stats immediately. It is left for the application to get
the pending measurements consumed, when it will not interfere with any
time-sensitive activities.
(E.g. if the application has a well-defined main loop, such as a game,
it is recommended to do the updates once every main loop iteration.)

The pending measurement data can be processed using `IPROF_SYNC`.

```C++
while (!quit)
{
	do_various_measured_things();

	IPROF_SYNC;
}

std::cerr << "Profiling stats:\n" << IPROF_STATS << '\n`;

```

### Multithreading

Unless disabled, iprof can also handle gathering stats across multiple threads, like:

```C++
IPROF_SYNC;
IPROF_SYNC_THREAD;
std::cerr << "The latest profiling stats across all threads:\n"
          << IPROF_ALL_THREAD_STATS << '\n';
```

In case some other threads might be still accumulating results, use the ```iProf::allThreadStatLock```
mutex to guard `IPROF_ALL_THREAD_STATS`:

```C++
{
    std::lock_guard<std::mutex> bouncer(iProf::allThreadStatLock);
    std::cerr << "The latest profiling stats across all threads:\n"
              << IPROF_ALL_THREAD_STATS << '\n';
}
```

For more guidance please see `test.cpp`.

## Building

Just add the headers and `iprof.cpp` to your project.

To build the provided example (`test.cpp`) with CLANG:

```bash
clang++ -std=c++17 -O3 -Wall test.cpp iprof.cpp
```

You can of course use `g++` instead, if you wish.

With MSVC:

```cmd
cl -EHsc -std:c++17 -O2 -W4 test.cpp iprof.cpp
```

(An MSVS project to build the example was also provided by the [original upstream repo](https://gitlab.com/Neurochrom/iprof), under its `winBuild` directory.)

### Options

- By defining `IPROF_DISABLE` (before including iprof.hpp) all of iprof will be omitted form the compilation; the `IPROF_...` macros will be defined empty (still passing through their arguments, though, so compilation without iprof would not introduce difficult-to-find subtle bugs).

- You can disable the multithreading functionality by defining `IPROF_DISABLE_MULTITHREAD`.

- In case your scopes are nested way too deep (more than 15), you might want to disable the constant-length vector optimization with
`IPROF_DISABLE_VECTOR_OPT`. You should also call the police and turn yourself in.

- You can include and use `hirestime.hpp` sepatately, on its own, if you only need a platform-independent high-resolution timer, but not the rest of iProf.

## Contributing

See the upstream repos:

- https://gitlab.com/Neurochrom/iprof (OG Paweł Cichocki)
- https://github.com/seanballais/iprof (CMake-ified version by Sean Francis Ballais)

## License

This library is **licensed under the (permissive) [MIT](https://opensource.org/licenses/MIT) license**.

Copyright (c) 2015-2019 Paweł Cichocki,
Copyright (c) 2023 Szabolcs Szász

Enjoy ;)
