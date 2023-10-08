#ifndef _SZ_TOGGLE_HH_
#define _SZ_TOGGLE_HH_

//#include <ostream> // The conv. op. should be enough!
#include <cassert>

namespace sz {

struct Toggle
{
	Toggle(bool s = false) : state(s) {}

	bool get()        const { return state; }
	operator bool()   const { return get(); }
	bool operator()() const { return get(); }

	void set(bool s)        { state = s; }
	void operator()(bool s) { set(s); }
	bool toggle()           { set(!get()); return get(); }

protected:
	bool state;
};

inline bool toggle(Toggle* t) { assert(t); return t->toggle(); }

} // namespace
#endif // _SZ_TOGGLE_HH_