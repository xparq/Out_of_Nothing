#include "hud.hpp"

//#include <typeinfo>
#include <string>
	using std::string, std::getline;
#include <string_view>
	using std::string_view;
#include <sstream>
	using std::stringstream;
#include <limits>
	using std::numeric_limits; // # of digits for precise float output
//#include <format>
#include <iostream>
	using std::cerr, std::endl;

using namespace std;
using namespace UI;

//!! Should be moved further out into some separate cpp_type_bullshit_..._something object instead!
//!! Also, not sure these could be auto, due to subtle const char* vs. char[...] mismatches...
// These have their own add(...):
/*static*/ const HUD::_nametype_ HUD::charptr_literal_name = "string literal";
/*static*/ const HUD::_nametype_ HUD::fptr_name     = "fptr";     // synthetic name for directly (*f)() compatible raw fn ptrs
/*static*/ const HUD::_nametype_ HUD::functor_name  = "CALLBACK"; // synthetic name for <CALLBACK> functor HUD::objects
// For the generic add():
/*static*/ const HUD::_nametype_ HUD::float_name   = typeid(float).name();
/*static*/ const HUD::_nametype_ HUD::double_name  = typeid(double).name();
/*static*/ const HUD::_nametype_ HUD::int_name     = typeid(int).name();
/*static*/ const HUD::_nametype_ HUD::bool_name    = typeid(bool).name();
/*static*/ const HUD::_nametype_ HUD::char_name    = typeid(char).name();
/*static*/ const HUD::_nametype_ HUD::charptr_name = typeid(const char*).name(); //!!??
/*static*/ const HUD::_nametype_ HUD::string_name  = typeid(string).name();

void HUD::_static_init()
{
/*!! So, the above init exprs. are not constexpr, but "const enough" to appear there. OK...
	static bool done = false; if (done) return; else done = true;
	charptr_literal_name = "string literal"; //!!?? Why did this work (why different from const char*): typeid("literal").name()?
	fptr_name     = "fptr";     // synthetic name for directly (*f)() compatible raw fn ptrs
	functor_name  = "CALLBACK"; // synthetic name for <CALLBACK> functor objects

	float_name   = typeid(float).name();
	double_name  = typeid(double).name();
	int_name     = typeid(int).name();
	bool_name    = typeid(bool).name();
	char_name    = typeid(char).name();
	charptr_name = typeid(const char*).name(); //!!??
	string_name  = typeid(string).name();
!!*/
}

//----------------------------------------------------------------------------
void HUD::add(const char* literal)
{
//cerr << "---> HUD: ADDING literal: "<<literal<<"\n";
	any noptr; noptr.emplace<void*>(nullptr);
	watchers.push_back(make_tuple(charptr_literal_name, noptr, literal));
}

void HUD::add(FPTR f)
{
//std::cerr << "adding " << fptr_name << ": "
//		     << (void*)f << " -> " << f() << endl;
	return add("", (void*)f, fptr_name);
//!!??Alas, this crashes:
//!!??		return add("", f, fptr_name);

//		any ptr; ptr.emplace<void*>(f); //!!?? and why does <FPTR>(f) crash here??
//		watchers.push_back(make_tuple(fptr_name, ptr, default_prompt));
}

//----------------------------------------------------------------------------
std::ostream& operator <<(std::ostream& out, const UI::HUD::WATCHER& d)
{
	// "Data watcher" tuples:
	//	get<0>: type
	//	get<1>: ref (address of the value)
	//	get<2>: prompt
	enum { Type, Ref, Prompt };

	const string_view prompt = get<Prompt>(d);
	const string_view type_name = get<Type>(d);

//cerr << "static HUD::charptr_name:         " << HUD::charptr_name << "         --- retrieved type_name: " << type_name << "\n";
//cerr << "static HUD::charptr_literal_name: " << HUD::charptr_literal_name << " --- retrieved type_name: " << type_name << "\n";
//cerr << "static HUD::float_name:           " << HUD::float_name << "           --- retrieved type_name: " << type_name << "\n";

#define _PTR(type) ( any_cast<type>(get<Ref>(d)) )

	try {
		//!! This is still too fragile! Remember: name() is implementation-dependent, and
		//!! could be anything: duplicates(!), mangled, long raw template name (even for string) etc.
		//!! E.g. with MSVC char* and char would both be just "char"! :-o
	  
		// Print both the prompt and the value...
		// (Done this way so that an exception can abort printing the entire object,
		// with the prompt too, not just the value.)

		if (type_name == HUD::charptr_literal_name) {
			// A const char* literal is just the "prompt", with no value. (Cf.: charptr_name!)
			out << prompt;
		} else if (type_name == HUD::charptr_name) { //!!??
//cerr << "GOT 'charptr_name' ("<< type_name <<")\n";
	//!!WAS:	out << (const char*) (* _PTR(const char**)) << '\n';
	//!!??		out << _PTR(const char*) << '\n';
	//!!Why exactly does this need const?! Would crash with <char**>, omitting it:
	//!!		prompt = (const char*) (* _PTR(char**));
		} else if (type_name == HUD::fptr_name) { // raw fn*
			auto val = (HUD::FPTR)_PTR(void*);
			out << prompt << val;
		} else if (type_name == HUD::functor_name) { // CALLBACK functor
			auto val = _PTR(HUD::CALLBACK)();
			out << prompt << val;
		} else if (type_name == HUD::int_name) {
			auto val = *_PTR(int*);
			out << prompt << val;
		} else if (type_name == HUD::bool_name) {
			auto val = (*_PTR(bool*) ? "on" : "off");
			out << prompt << val;
		} else if (type_name == HUD::float_name) {
			auto save = out.precision(numeric_limits<float>::max_digits10); //!! Did I leave max as a placeholder?
			auto val = *_PTR(float*);
			out << prompt << val;
			out.precision(save);
		} else if (type_name == HUD::double_name) {
			auto save = out.precision(numeric_limits<double>::max_digits10); //!! Did I leave max as a placeholder?
			auto val = *_PTR(double*);
			out << prompt << val;
			out.precision(save);
		} else if (type_name == HUD::string_name) {
			auto val = *_PTR(const string*);
			out << prompt << val;
		} else if (type_name == HUD::char_name) { // Plain char value (not char*!)
			auto val = *_PTR(char*);
			out << prompt << val;
		} else {
			out << "ERROR: Can't show unknown type: " << get<Type>(d);
		}
		out << "\n";

	} catch(std::bad_any_cast&) {
cerr << "- ERROR: Type mismatch for \"" <<get<Prompt>(d)<< "\", with " <<type_name<< "!\n";
		// Nothing added to 'out', continuing...
	} catch(...) {
cerr << "- ERROR: Wow, unknown exception in " __FUNCTION__ "!\n";
	}
	
#undef _PTR

	return out;
}
