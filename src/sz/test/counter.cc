#include "sz/counter.hh"
#include <cstdint>
#include <iostream>

using namespace sz;
using namespace std;

int main()
{
	Counter<int> x = 1;
	cerr << "Counter<int> x = 1; Should be 1: " << x << "\n";

	//-------------------------------------------------------------------
	GuardedCounter<uint8_t> gc(255);
	gc.on_overflow = [](auto& c) { cerr << "overflowed! " << unsigned(c) << endl; }; // unsigned(c), as <<uint8 is botched in C++! :-/
	gc++;

	//-------------------------------------------------------------------
	if (gc == 0) cerr << "OK, comparison works, too, if this was printed.\n";

	cerr << "Comparison: Counter<>(1) < Counter<>(2) ?\t";
		cerr << boolalpha << ( Counter<>(1) < Counter<>(2) ) << endl;

	//-------------------------------------------------------------------
	cerr << "\nfor (Counter i; i++ <= 10;) ... -> 1 .. 10\n";
		for  (Counter<> i; i++ <= 10;) cerr << i << " ";

	cerr << "\n\nfor (Counter i(8); i < 10; ++i) ... -> 8 .. 9\n";
		for  (Counter<> i(8); i < 10; ++i) cerr << i << " ";

	cerr << "\n\n"
	     << "Capped<>: default max (unsigned): " << CappedCounter<>().max() << "\n";
	cerr << "Capped<uint8_t>: default max: " << (unsigned)CappedCounter<uint8_t>().max() << "\n";
	cerr << "Capped<short>: default max: " << CappedCounter<short>().max() << "\n";
	cerr << "Capped<size_t>: default max: " << CappedCounter<size_t>().max() << "\n";

	cerr << "\nCapped: 0 .. 10!\n\t";
		CappedCounter<> cc(10);
		for (int i = 0; i++ < 20; ++cc)	cerr << cc << (cc.maxed()?"!":"") << " ";

	cerr << "\nCapped: 5 .. 20!\n\t";
		CappedCounter<> cc2(20, 5);
		for (int i = 0; i++ < 20; cc2++) cerr << cc2 << (cc2.maxed()?"!":"") << " ";

	cerr << "\nReuse (reset, recap): 0 .. 13!\n\t";
		cc.reset(); cc.max(13);
		for (int i = 0; i++ < 15; cc++) cerr << cc << (cc.maxed()?"!":"") << " ";

	//-------------------------------------------------------------------
	cerr << "\n\nAssign: operator=(-1u)\n\t";
		cc.operator=(unsigned(-1)); cerr << cc << ", ";
	cerr << "\nAssign: x = -1u\n\t";
		cc = unsigned(-1); cerr << cc << ", ";

	cerr << "\n\nAssign: operator=(-1)\n\t";
		cc.operator=(unsigned(-1)); cerr << cc << ", ";
	cerr << "\nAssign: x = -1\n\t";
		cc = unsigned(-1); cerr << cc << ", ";

	cerr << "\n\nAssign: copy op= cc2 (should be 20)\n\t";
		cc = cc2; cerr << cc << ", ";

	cerr << "\n\nAssign: funcall-op(7)\n\t";
		cerr << cc(7) << "\n";
}
