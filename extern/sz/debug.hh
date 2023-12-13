// v0.0.1

#ifndef _C08Y7T42708BFC2308745Y870NF7803226Y435BVN7_
#define _C08Y7T42708BFC2308745Y870NF7803226Y435BVN7_

#include <string>
#include <string_view>
#include <iostream>

namespace sz {

struct SCOPE_TRACER
{
	std::string _scopename;
	SCOPE_TRACER(std::string scopename = "") : _scopename(scopename)
		{ std::cerr << (_scopename.empty() ? "" : _scopename + " { // BEGIN") << "" << '\n'; }
	~SCOPE_TRACER()
		{ std::cerr << (_scopename.empty() ? "" : _scopename + " } // END") << "" << '\n'; }
};

struct DEBUG_DUMPER
{
	std::ostream& out;
	DEBUG_DUMPER(std::ostream& out = std::cerr) : out(out) { out << "DBG> "; }
	DEBUG_DUMPER& operator ,(std::string_view str) { out << str; }
	~DEBUG_DUMPER() { out << '\n'; }

	//!!...
};

} // namespaces sz

#ifdef DEBUG

#  define _        sz::SCOPE_TRACER _scopeguard__add__COUNTER__(__FUNCTION__);
#  define __(x)    sz::SCOPE_TRACER _scopeguard__add__COUNTER__(#x);
//!!#  define DBG(...) DEBUG_DUMPER(...)

#else // DEBUG

#  define _     ;
#  define __(x) ;

#endif // DEBUG

#endif // _C08Y7T42708BFC2308745Y870NF7803226Y435BVN7_
