// v0.1.2

#ifndef _8BNVCX8567245872V85BN78456NBV8C_
#define _8BNVCX8567245872V85BN78456NBV8C_

//!!#include <ostream> //!! For defining our own >> op, as the std >>
                       //!! (templates!) won't see our op. bool! :-/
#include <cassert>

namespace sz {

struct Toggle
{
	Toggle(bool s = false) : state(s) {}

	bool toggle()            { set(!get()); return get(); }

	bool         get() const { return state; }
	virtual void set(bool s) { state = s; }

	// Get/set the natural way:
	operator bool()    const { return get(); }
	bool operator=(bool s)   { set(s); return s; }

	// Get/set with name()/name(newstate):
	bool operator()()  const { return get(); }
	bool operator()(bool s)  { set(s); return s; }

protected:
	bool state;
};

// A "generic toggler", to uniformly toggle any bool...
inline bool toggle(Toggle& t) { return t = !t; }
inline bool toggle(Toggle* t) { assert(t); return toggle(*t); }

} // namespace sz
#endif // _8BNVCX8567245872V85BN78456NBV8C_



//============================================================================
#ifdef UNIT_TEST

#include <iostream>

using namespace sz;
using namespace std;

int main()
{
	Toggle t;

//!! Wow, left-to-right eval. order has only been guaranteed here since c++17!... :-o
//!!	cout <<"t: "<< t <<", t.get(): "<< t.get() <<", toggled: "<< toggle(&t) <<'\n';
	cout <<"t: "<< t <<", t.get(): "<< t.get() <<", toggled: ";cout<< toggle(&t) <<'\n';

	cout <<"Using sz::toggle(&t)... Before: "<< t <<", after: ";cout<< toggle(&t) <<'\n';
}

#endif
