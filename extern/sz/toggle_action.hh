// v0.0.1

#ifndef _DN8CSVBUYTTBVYUC78HN02807XNXZ989485_
#define _DN8CSVBUYTTBVYUC78HN02807XNXZ989485_

//----------------------------------------------------------------------------
// Make sure the other headers won't want to add their own unit tests...
#ifdef UNIT_TEST
# define save_UNIT_TEST_DN8CSVBUYTTBVYUC78HN02807XNXZ989485_ UNIT_TEST
# undef UNIT_TEST
#endif

#include "toggle.hh"

#ifdef save_UNIT_TEST_DN8CSVBUYTTBVYUC78HN02807XNXZ989485_
# define UNIT_TEST save_UNIT_TEST_DN8CSVBUYTTBVYUC78HN02807XNXZ989485_
# undef save_UNIT_TEST_DN8CSVBUYTTBVYUC78HN02807XNXZ989485_
#endif
//----------------------------------------------------------------------------

#include <functional>

namespace sz {

struct ToggleAction : public Toggle
{
	ToggleAction(std::function<void(bool)> callback, bool s = false) :
		Toggle(s), callback(callback) { callback(state); }

	void set(bool s) override { state = s; callback(s); }

protected:
	std::function<void(bool)> callback;
};

} // namespace sz


#endif // _DN8CSVBUYTTBVYUC78HN02807XNXZ989485_


//============================================================================
#ifdef UNIT_TEST

#include <iostream>

using namespace sz;
using namespace std;

int main()
{
	ToggleAction t([](bool state){ cout <<" [Callback for "<< state <<"!] "; });

//!! Wow, left-to-right eval. order has only been guaranteed here since c++17!... :-o
//!!	cout <<"t: "<< t <<", t.get(): "<< t.get() <<", toggled: "<< toggle(&t) <<'\n';
	cout <<"t: "<< t <<", t.get(): "<< t.get() <<", toggled: ";cout<< toggle(&t) <<'\n';

	cout <<"Using sz::toggle(&t)... Before: "<< t <<", after: ";cout<< toggle(&t) <<'\n';

//!! Not possible with C++, AFAIK:
//!!	cout <<"Closure accessing the toggle and forcing state to true:\n";
//	ToggleAction tx([&tx](bool state){ cout <<" [Callback for itself!] "; tx.Toggle::set(true); });
//	cout << tx <<'\n';
//	cout << tx.toggle() <<'\n';
}

#endif
