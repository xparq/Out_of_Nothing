#include "Object.hpp"
#include "misc/stringtools.hpp"

//#include <cassert>
#include <fstream>
	using std::ofstream, std::ifstream, std::ios;
#include <string>
	using std::string;
//#include <cstddef>
//	using std::byte; //!!No use: ofstream can't write() bytes! :-o Congratulations... :-/
#include <iostream>
	using std::cerr, std::endl;

namespace Model {

//!!?? Put a EventSubscriber* pointer here to the global OON_* instance to help propagating events?!
//!!   ...That would require an EventSubscriber interface first, also then actually used by OON. ;)


//============================================================================
void World::Body::recalc()
{
	mass = powf(r, 3) * density;
//	Physics::BV_to_T_and_RGB(Physics::T_to_BV(T), superpower.free_color ? nullptr : &color);
	Physics::T_to_RGB_and_BV(T, superpower.free_color ? nullptr : &color);
}

//----------------------------------------------------------------------------
void World::Body::on_event(Event e, ...)
{
	r *= 0.1f; //!!Just to see if it works at all!...
}



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
	//!! initializing a template object from that.
	//!!

	//!!out.write(BSIG, sizeof(BSIG));
	string memdump(reinterpret_cast<char*>(this), sizeof(*this));
	misc::escape_quotes(memdump);
	try {
		out.write(memdump.data(), memdump.size());
		//!!?? if (out.bad()) {...
	} catch (...) {
		cerr << "- ERROR: Failed to write to the output stream!\n";
		return false;
	}
	return true;
}

World::Body World::Body::load(std::istream& in) // static (factory)
{
	return World::Body();
}

}; // namespace
