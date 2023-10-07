//----------------------------------------------------------------------------
// Tiny, fixed-size, "lossy" POD container for an std::vector replacement
// optimized for use in maps as a tag-list (path) key
//
// Lossy in that if full, newly added elements are silently ignored, while
// size() still pretends to grow (so at least the intended size is preserved).
// (Use capacity() to query the actual # of filled slots.)
//
// POD-only (or "trivially-copyable-only"?): memcmp is used for comparisons.
// (No deep cloning either: copying the container means trivial shallow copy.)
//----------------------------------------------------------------------------
// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT
//----------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <string.h>

#if defined(DEBUG)
# include <type_traits> // is_same
# include <algorithm> // min
# include <iostream> // cerr
#endif

template <typename T, unsigned MAX_SIZE = 15>
class LossyVector
{
public:
	typedef uint16_t size_type;
	typedef T value_type;

protected:
	value_type items[MAX_SIZE];
	size_type fill = 0;      // # of actually contained items
	size_type virtsize = 0;  // can be > fill, and can even overflow

public:
	constexpr size_type size() const
	{
		return virtsize;
	}

	constexpr size_type capacity() const
	{
		return fill;
	}

	constexpr void push_back(value_type item)
	{
		++virtsize;
		if (fill < MAX_SIZE)
			items[fill++] = item;
		return;
	}

	constexpr void pop_back()
	{
		if (--virtsize < MAX_SIZE)
			--fill;
	}

	constexpr const value_type* begin() const { return items; }
	constexpr const value_type* end() const { return items + fill; }
	constexpr const value_type& front() const { return *items; }
	constexpr const value_type& back() const { return *(items + fill - 1); }

	bool operator==(const LossyVector& a) const
	{
		return size() == a.size() && 0 == memcmp(items, a.items, sizeof(value_type) * fill);
	}

	bool operator<(const LossyVector& a) const
	{
		if (size() < a.size())
			return true;
		else if (size() > a.size())
			return false;
		return memcmp(items, a.items, sizeof(value_type) * fill) < 0;
	}

	//! NOTE: If T == const char* (or any pointer), we are still just
	//!       comparing pointers above!
	//!
	//! The usefulness of this depends on the compiler putting all the
	//! identical const char* string literals under the same pointer,
	//! which tends to happen in practice, but is not guaranteed by
	//! the std., of course.
	//! E.g. calling push_back("...") from inline functions in multiple
	//! transl. units might increase the chance of this being a problem.

#if defined(DEBUG)
	~LossyVector()
	{
		if constexpr (std::is_same_v<T, const char*>)
		{
			thread_local bool done = false; if (done) return; else done = true;
			constexpr size_type CHECK_MAX = min(CHECK_MAX, 100);
			// strcmp all combinations of unordered pairs (up to CHECK_MAX),
			// to see if there are strings that compare the same despite !op==,
			// and report (assert) them!
			std::cerr << "\n[LossyVector: Searching for duplicate strings...]\n";
			for (size_type i = 0; i < std::min(capacity(), CHECK_MAX); ++i)
				for (size_type j = i + 1; j < std::min(capacity(), CHECK_MAX); ++j)
					// Alas, assert() can't print those strings...
					if (!(items[i] != items[j] && items[i] && items[j] &&
					       0 == strcmp(items[i], items[j])))
					       std::cerr << "[LossyVector: items["<<i<<"] != items["<<j<<"] for the same strings: "
					            << items[i] << " == " << items[j] << '\n';
		}
	}
#endif
};
