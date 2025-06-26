#include "Error.hpp"
#include "Log.hpp"

#include <sstream>
#include <iostream>
using namespace std;


//!!
//!! NOTE:
//!!
//!! - Now the source location is blatantly included in the user-facing messages too, but that shouldn't be the case!
//!! - However, the actual location *can't* be forwarded to the LOG macros, which would just grab the fixed locations here,
//!!   so, as a workaround, the orig. locations are embedded into the log messages themselves! :-/
//!! - Also, we assume here that the logger _may_ add its own ERROR/WARNING etc. prefixes or severity tags (risking omission).
//!! - Similarly, we assume that the logger _may not_ add the source location already (so we do it here, risking duplication).
//!!


namespace Szim {

namespace {
	auto src_loc_suffix = [](const SourceLocation& loc, const char* preposition = "at") {
		return ( ostringstream()
			<< " ("<<preposition <<" "
			<< loc.fn << ":" << loc.ln
			<< ")"
		).str();
	};
}

void NOTE_impl (const SourceLocation& loc, std::string_view message, ...)
// The source location is omitted from the  write the location!
{
//	auto msg = ostringstream()
//		<< message << src_loc_suffix(loc);

	LOGN	<< message << src_loc_suffix(loc);
	cerr	<< "Note: " << message << endl;
}

void WARNING_impl (const SourceLocation& loc, std::string_view message, ...)
{
	auto msg = ostringstream()
		<< message << src_loc_suffix(loc);

	LOGW	<< msg.str();
	cerr	<< "Warning: " << msg.str() << endl;
}

void ERROR_impl   (const SourceLocation& loc, std::string_view message, ...)
{
	auto msg = ostringstream()
		<< message << src_loc_suffix(loc);

	LOGE	<< msg.str();
	cerr	<< "- ERROR: " << msg.str() << endl;
}

void FATAL_impl   (const SourceLocation& loc, std::string_view message, ...)
{
	auto msg = ostringstream()
		<< message << src_loc_suffix(loc);

	LOGF	<< msg.str();
	cerr	<< "- FATAL ERROR: " << msg.str() << endl;

	ABORT();
}

void BUG_impl   (const SourceLocation& loc, std::string_view message, ...)
{
	auto msg = ostringstream()
		<< "\n" // Don't just assume we're on a new line already; break away forcefully
		        // (at the cost of an extra empty line in the most common case)...
		<< "**************** INTERNAL ERROR" << src_loc_suffix(loc) << ":\n"
		<< message << "\n"
		<< "****************\n";

	LOGE	//!!<< "\n" // Guarantee an empty line after a possible "ERROR: " log entry prefix
		//!!        // — but there would be *two* empty lines if no prefix; awkward!...
		<< msg.str();

	cerr	<< msg.str() << endl;
}

void ABORT_impl   (const SourceLocation& loc, std::string_view message, ...)
{
	const char* prefix = "--- ABORTING";
	if (message != "") {
		if (loc) {
			LOG	<< message << src_loc_suffix(loc);
			cerr	<< prefix << ": " << message << src_loc_suffix(loc) << endl;
		} else {
			LOG	<< message;
			cerr	<< prefix << ": " << message << endl;
		}
	} else {
		LOG	<< prefix << "...";
		cerr	<< prefix << "..." << "\n";
	}
	// Well, this requires a try{} block around main, _with the appropriate catch(Szim::FatalError& x) {}_, too! :-/ 
	throw FatalError(message, loc);
}

} // namespace Szim