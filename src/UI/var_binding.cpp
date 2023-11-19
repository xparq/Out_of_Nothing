#include "var_binding.hpp"

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
/*static*/ const Binding::_nametype_ Binding::charptr_literal_name = "string literal";
/*static*/ const Binding::_nametype_ Binding::fptr_name     = "fptr";     // synthetic name for directly (*f)() compatible raw fn ptrs
/*static*/ const Binding::_nametype_ Binding::functor_name  = "CALLBACK"; // synthetic name for <CALLBACK> functor Binding::objects
// For the generic add():
/*static*/ const Binding::_nametype_ Binding::float_name   = typeid(float).name();
/*static*/ const Binding::_nametype_ Binding::double_name  = typeid(double).name();
/*static*/ const Binding::_nametype_ Binding::int_name     = typeid(int).name();
/*static*/ const Binding::_nametype_ Binding::bool_name    = typeid(bool).name();
/*static*/ const Binding::_nametype_ Binding::char_name    = typeid(char).name();
/*static*/ const Binding::_nametype_ Binding::charptr_name = typeid(const char*).name(); //!!??
/*static*/ const Binding::_nametype_ Binding::string_name  = typeid(string).name();

/*!! So, the above init exprs. are not constexpr, but "const enough" to appear there. OK...
void Binding::_static_init()
{
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
}
!!*/

//----------------------------------------------------------------------------
Binding::Binding(const char* literal)
{
//cerr << "---> HUD: ADDING literal: "<<literal<<"\n";
	any noptr; noptr.emplace<void*>(nullptr);
	_binding = make_tuple(charptr_literal_name, noptr, literal);
}

Binding::Binding(FPTR f) :
	Binding("", (void*)f, fptr_name)
{
//std::cerr << "adding " << fptr_name << ": "
//		     << (void*)f << " -> " << f() << endl;
//!!??Alas, this crashes:
//!!??		return add("", f, fptr_name);

//		any ptr; ptr.emplace<void*>(f); //!!?? and why does <FPTR>(f) crash here??
//		watchers.push_back(make_tuple(fptr_name, ptr, default_prompt));
}

//----------------------------------------------------------------------------
std::ostream& operator <<(std::ostream& out, const UI::Binding& d)
{
	// "Data watcher" tuples:
	//	get<0>: type
	//	get<1>: ref (address of the value)
	//	get<2>: prompt
	enum { Type, Ref, Prompt };

	const string_view prompt = d.prompt();
	const string_view type_name = d.type_name();

//cerr << "static Binding::charptr_name:         " << Binding::charptr_name << "         --- retrieved type_name: " << type_name << "\n";
//cerr << "static Binding::charptr_literal_name: " << Binding::charptr_literal_name << " --- retrieved type_name: " << type_name << "\n";
//cerr << "static Binding::float_name:           " << Binding::float_name << "           --- retrieved type_name: " << type_name << "\n";

#define _PTR(type) ( any_cast<type>(d.ref()) )

	try {
		//!! This is still too fragile! Remember: name() is implementation-dependent, and
		//!! could be anything: duplicates(!), mangled, long raw template name (even for string) etc.
		//!! E.g. with MSVC char* and char would both be just "char"! :-o
	  
		// Print both the prompt and the value...
		// (Done this way so that an exception can abort printing the entire object,
		// with the prompt too, not just the value.)

		if (type_name == Binding::charptr_literal_name) {
			// A const char* literal is just the "prompt", with no value. (Cf.: charptr_name!)
			out << prompt;
		} else if (type_name == Binding::charptr_name) { //!!??
//cerr << "GOT 'charptr_name' ("<< type_name <<")\n";
	//!!WAS:	out << (const char*) (* _PTR(const char**)) << '\n';
	//!!??		out << _PTR(const char*) << '\n';
	//!!Why exactly does this need const?! Would crash with <char**>, omitting it:
	//!!		prompt = (const char*) (* _PTR(char**));
		} else if (type_name == Binding::fptr_name) { // raw fn*
			auto val = (Binding::FPTR)_PTR(void*);
			out << prompt << val;
		} else if (type_name == Binding::functor_name) { // CALLBACK functor
			auto val = _PTR(Binding::CALLBACK)();
			out << prompt << val;
		} else if (type_name == Binding::int_name) {
			auto val = *_PTR(int*);
			out << prompt << val;
		} else if (type_name == Binding::bool_name) {
			auto val = (*_PTR(bool*) ? "on" : "off");
			out << prompt << val;
		} else if (type_name == Binding::float_name) {
			auto save = out.precision(numeric_limits<float>::max_digits10); //!! Did I leave max as a placeholder?
			auto val = *_PTR(float*);
			out << prompt << val;
			out.precision(save);
		} else if (type_name == Binding::double_name) {
			auto save = out.precision(numeric_limits<double>::max_digits10); //!! Did I leave max as a placeholder?
			auto val = *_PTR(double*);
			out << prompt << val;
			out.precision(save);
		} else if (type_name == Binding::string_name) {
			auto val = *_PTR(const string*);
			out << prompt << val;
		} else if (type_name == Binding::char_name) { // Plain char value (not char*!)
			auto val = *_PTR(char*);
			out << prompt << val;
		} else {
			out << "ERROR: Can't show unknown type: " << d.type_name();
		}
		out << "\n";

	} catch(std::bad_any_cast&) {
cerr << "- ERROR: Type mismatch for \"" <<d.prompt()<< "\", with " <<type_name<< "!\n";
		// Nothing added to 'out', continuing...
	} catch(...) {
cerr << "- ERROR: Wow, unknown exception in " __FUNCTION__ "!\n";
	}
	
#undef _PTR

	return out;
}
