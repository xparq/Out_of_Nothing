#ifndef _SZ_COUNTER_HH_
#define _SZ_COUNTER_HH_

#include <limits>
#include <functional>
//#include <ostream> // The conv. ops. should be enough!
#include <cassert>

namespace sz {

//--------------------------------------------------------------------------------------
template <typename CountType = unsigned>
struct Counter
// Has the same interval and overflow characteristics as CountType
{
private:
	using Self = Counter<CountType>;
public:
	constexpr Counter(CountType from = 0) : count(from) {}

	constexpr CountType get()        const { return count; }
	constexpr operator CountType()   const { return get(); }
	constexpr CountType operator()() const { return get(); }

	constexpr void reset()                      { count = 0; }
	constexpr void set(CountType n)             { count = n; }
	constexpr CountType operator()(CountType n) { set(n); return n; }
	Self&     operator=(CountType n)       { set(n); return *this; }
	Self&     operator=(const Self& other) { set(other.count); return *this; }
	constexpr CountType operator++(int)    { return ++count; }
	constexpr CountType operator++()       { return count++; }
	constexpr CountType operator--(int)    { return --count; }
	constexpr CountType operator--()       { return count--; }
	//auto operator<=>(const Counter<CountType& other) { return count <=> other.count; }
		// Not needed: the ones for CountType would be found via the conv. op.

//protected:
	CountType count;
};


//--------------------------------------------------------------------------------------
template <typename CountType = unsigned>
struct CappedCounter : Counter<CountType>
// Stops at the max. val. on ++, but -- is not guarded (behaves the same as CountType)
// The other modifiers aren't checked either, and maxed() is undefined if count > max.
{
private:
	using Self = CappedCounter<CountType>;
	using Base = Counter<CountType>;
public:
	constexpr
	CappedCounter(CountType max = std::numeric_limits<CountType>::max(),
	              CountType from = 0)
		: Base(from), limit(max) {}

	Self&     operator=(CountType n)       { Base::set(n); return *this; }
	Self&     operator=(const Self& other) { Base::set(other.count); return *this; }
	constexpr void      max(CountType max) { limit = max; }
	constexpr CountType max()        const { return limit; }
	constexpr CountType maxed()      const { return Base::get() == limit; }
	constexpr CountType operator++(int)    { return maxed() ? Base::get() : Base::operator++(0); }
	constexpr CountType operator++()       { return maxed() ? Base::get() : Base::operator++(); }

//protected:
	CountType limit;
};


//--------------------------------------------------------------------------------------
template <typename CountType = unsigned>
struct GuardedCounter : Counter<CountType>
// Overflow-Guarded variant, with a callback and an overflow (carry) flag
// The callback receives the instance, and is expected to clear the overflow flag.
{
private:
	using Self = GuardedCounter<CountType>;
	using Base = Counter<CountType>;
protected:
	constexpr void _guarded_inc() { auto oldcount = Base::get(); Base::operator++(0);
	                                 if (oldcount > Base::get()) { overflowed = true; on_overflow(*this); } }
public:
	using Base::Base; // Use the upstream ctors

	Self&     operator=(Self other)        { Base::set(other.count); return *this; }
	Self&     operator=(const Self& other) { Base::set(other.count); return *this; }
	constexpr CountType operator++(int)    { _guarded_inc(); return Base::get(); }
	constexpr CountType operator++()       { auto oldcount = Base::get(); _guarded_inc(); return oldcount; }

//protected:
	bool overflowed = false;
	std::function<void(Self&)> on_overflow = [](Self&){}; // Another version could just throw instread...
};

} // namespace
#endif // _SZ_COUNTER_HH_
