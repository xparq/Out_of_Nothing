// 0.0.32

#ifndef _RINGBUFFER_DX298E76Y5FNE78573X8H7HY_
#define _RINGBUFFER_DX298E76Y5FNE78573X8H7HY_

#include <cstddef> // size_t
//#include <cassert>

template <typename T, std::size_t Capacity, T ErrorValue>
//!!TODO: Prevent Capacity == 0
class RingBuffer_Static_Int // vanilla copy semantics!
{
// This implementation is only efficient for small (copyable) integral types.
// (Reference semantics would insist on using pointers (even with -Os), so
// that's for bigger types.)
//
// Error handling policy:
//
// The user should check for empty() and full() before adding/removing.
//
//!!TODO: Change this to an optional template trait, defaulting to the normal STL thing!
// But for extra robustness, run-time error checking is now also implemented,
// via a mandatory dedicated error value, which is reserved from the value set
// and returned in case of invalid calls.
public:
	T           _items[Capacity];
	std::size_t _next, _oldest, _count;


	RingBuffer_Static_Int() {
		clear();
	}


	// Add new to "end"
	void push_back(T x) {
		// Drop the oldest if full:
		if (full()) pop_front();

		__set_at(_next, x);
		__i_inc(_next);
		++_count;
	}

	// Remove oldest from "begin"
	T pop_front() {
		if (empty()) return ErrorValue;

		T x = __get_at(_oldest);
		--_count;
		__i_inc(_oldest);
		return x;
	}

	// Remove newest from "end"
	T pop_back() {
		if (empty()) return ErrorValue;

		__i_dec(_next);
		T x = __get_at(_next);
		--_count;
		return x;
	}

	T front() { return empty() ? ErrorValue : *__i_front(); }
	T back()  { return empty() ? ErrorValue : *__i_back(); }

	bool find(T value) const
	{
		if (!empty()) {
			auto i = __i_begin();
			do {
				if (__get_at(i) == value) {
					return true;
				} else {
					__i_inc(i);
				}
			} while (i != __i_end());
		}
		return false;
	}

	bool full()            const { return _count == Capacity; }
	bool empty()           const { return _count == 0; }
	std::size_t size()     const { return _count; }
	std::size_t capacity() const { return Capacity; }

	void clear() { _count = 0; _next = 0; _oldest = 0; }

/*
	T* getbuf_I_know_the_risks()
	// But remember: accessing the guts of a RingBuffer is a highly
	// shaky practice, and the caller will have full responsibility
	// to lifecycle and access management, as well as making sure that 
	// implicitly copy-constructing relocated items to and fro is fine...
	//!!Add move semantics!
	{
		__normalize();
		return &_items;
	}
*/

//!!TODO	Iterator...
protected:

	std::size_t __i_begin() const { return _oldest; }
	std::size_t __i_end()   const { return _next; }

	T*          __i_front() { return &(_items[_oldest]); }
	T*          __i_back()  { return _next > 0 ? &(_items[_next - 1]) : &(_items[Capacity - 1]); }

	//! Note: macros (with non-ref. args...) produced the same code. Phew. :)
	void __i_dec(std::size_t& i) { if (i > 0) --i; else i = Capacity - 1; }
	void __i_inc(std::size_t& i) { if (i < Capacity -1) ++i; else i = 0; }

	T    __get_at(std::size_t i) const    { return _items[i]; }
	void __set_at(std::size_t i, T value) {        _items[i] = value; }

/*
	void __normalize()
	// This facilitates using the raw buffer as a contiguous memory block
	// (i.e. a plain old C array).
	// But it's a problematic move... not just because it *is* a move...
	// See more at getbuf_I_know_the_risks()!
	{
		if (_count > 0 and _oldest > 0) {
			// Rotate the buffer through a temporary T back
			// #_oldest steps to square 1 (i.e. 0)...
			for (int i = 0; i < _count; ++i) {
				//!! Now what...? 8-o ;)
//http://stackoverflow.com/questions/21479784/shifting-aligning-rotating-a-circular-buffer-to-zero-in-place
//http://stackoverflow.com/questions/4457277/algorithm-to-rotate-an-array-in-linear-time
//http://www.cplusplus.com/reference/algorithm/rotate/
			}
		}
	}
*/

};

#endif // _RINGBUFFER_DX298E76Y5FNE78573X8H7HY_
