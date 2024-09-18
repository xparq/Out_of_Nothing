#include "Object.hpp"
#include "sz/str.hh"
#include "sz/DBG.hh"

#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#include <iomanip>
//	using std::quoted;
#include <string>
	using std::string;
//#include <cstddef>
//	using std::byte; //!!No use: ofstream can't write() bytes! :-o Congratulations, C++!... :-/
#include <cassert>
#include <iostream>
	using std::cerr, std::endl;


namespace Model {

//static constexpr char BSIG[] = {'O','B','0','1'};
//----------------------------------------------------------------------------
bool World::Body::save(std::ostream& out)
{
	//!!
	//!! This is totally brittle as yet, won't survive any change in the struct,
	//!! or 32/64 bit recompilation etc... Can't keep it this way for long!
	//!!
	//!! Note that just hex-encoding won't help at all (-> 32/64)!... ;)
	//!!
	//!! Short of integrating a good 3rd-party lib, perhaps the most robust
	//!! option could be loading whatever's saved into a std::map, and then
	//!! initializing a template object from that. [FUTURE ME HERE: I don't
	//!! unrestand this std::map idea any more... :-/ ]
	//!!

	//!!out.write(BSIG, sizeof(BSIG));
	string memdump(reinterpret_cast<char*>(this), sizeof(*this));
	sz::escape_quotes(&memdump); //! Prevent istream<< from messing up the load()...
	                             //!!?? Why exactly ios::binary not enough, again?
	try {
		out.write(memdump.data(), memdump.size());
		//!!?? if (out.bad()) {...
	} catch (...) {
		cerr << "- ERROR: Failed to write to the output stream!\n";
		return false;
	}
	return true;
} // save

//----------------------------------------------------------------------------
// A static factory method:
/*static*/ bool World::Body::load(std::istream& in, World::Body* result/* = nullptr*/)
{
	if (!result) {
		return false; //!! VERIFY NOT IMPLEMENTED YET!
	}

	string objdump;
	in >> std::quoted(objdump, '"', '\\'); //! This can properly read broken lines,
		//! and wouldn't need fixed "record sizes", but requires escaping any
		//! stray quotes in the object's mem dump -- which also means a string
		//! reallocation per every few dozen objects, BTW.
//cerr << "["<<ndx<<"]" << c <<" \""<< objdump << "\"" << endl;

	if (sizeof(Body) != objdump.size()) {
		cerr << "- ERROR: Failed to load object! Bytes expected: " << sizeof(Body) << ", found: " << objdump.size() <<".\n";
		return false;
	}

	memcpy((void*)result, objdump.data(), sizeof(Body));
//!!THIS IS BOGUS YET: THESE DIDN'T MATCH! :-o WTF?! :-ooo
//!!cerr << "template_obj.T = " << template_obj.T << endl;

	return true;
} // load

} // namespace Model