#include "Binding.hpp"

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
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::charptr_literal_name = "string literal";
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::fptr_name     = "fptr";     // synthetic name for directly (*f)() compatible raw fn ptrs
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::functor_name  = "CALLBACK"; // synthetic name for <CALLBACK> functor Binding::objects
// For the generic add():
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::float_name   = typeid(float).name();
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::double_name  = typeid(double).name();
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::int_name     = typeid(int).name();
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::bool_name    = typeid(bool).name();
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::char_name    = typeid(char).name();
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::charptr_name = typeid(const char*).name(); //!!??
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::string_name  = typeid(string).name();

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
Binding::Binding(const char* literal) :
	_data_ptr(literal),
	_type(Binding::charptr_literal_name)
{
//cerr << "---> HUD: ADDING literal: "<<literal<<"\n";
}

Binding::Binding(const std::string* literal) :
	_data_ptr(literal),
	_type(Binding::string_name)
{
//cerr << "---> HUD: ADDING literal: "<<literal<<"\n";
}


Binding::Binding(Binding::STRING_F_PTR f) :
	Binding((void*)f, fptr_name)
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
	const string_view type_name = d.type_name();

//cerr << "static Binding::charptr_name:         " << Binding::charptr_name << "         --- retrieved type_name: " << type_name << "\n";
//cerr << "static Binding::charptr_literal_name: " << Binding::charptr_literal_name << " --- retrieved type_name: " << type_name << "\n";
//cerr << "static Binding::float_name:           " << Binding::float_name << "           --- retrieved type_name: " << type_name << "\n";

#define _PTR(type) ( any_cast<type>(d.ref()) )

	//!!MOVE THIS ENTIRE CAST-BACK PART TO A TEMPLATED AUTO-CONVERSION MEMBER? (IF THAT'S POSSIBLE AT ALL)
	try {
		//!! This is still too fragile! Remember: name() is implementation-dependent, and
		//!! could be anything: duplicates(!), mangled, long raw template name (even for string) etc.
		//!! E.g. with MSVC char* and char would both be just "char"! :-o

		if (type_name == Binding::charptr_literal_name) {
			//!! Cf.: charptr_name!
			auto val = _PTR(const char*);
			out << val;
		} else if (type_name == Binding::charptr_name) { //!!??
//cerr << "GOT 'charptr_name' ("<< type_name <<")\n";
	//!!WAS:	out << (const char*) (* _PTR(const char**)) << '\n';
	//!!??		out << _PTR(const char*) << '\n';
	//!!Why exactly does this need const?! Did crash with <char**>, omitting it -- was it just bac_any_cast?
	//!!		prompt = (const char*) (* _PTR(char**));
		} else if (type_name == Binding::fptr_name) { // raw fn*
			auto val = (Binding::STRING_F_PTR)_PTR(void*);
			out << val;
		} else if (type_name == Binding::functor_name) { // CALLBACK functor
			auto val = _PTR(Binding::STRING_FUNCTOR)();
			out << val;
		} else if (type_name == Binding::int_name) {
			auto val = *_PTR(int*);
			out << val;
		} else if (type_name == Binding::bool_name) {
			auto val = (*_PTR(bool*) ? "on" : "off");
			out << val;
		} else if (type_name == Binding::float_name) {
			auto save = out.precision(numeric_limits<float>::max_digits10); //!! Did I leave max as a placeholder?
			auto val = *_PTR(float*);
			out << val;
			out.precision(save);
		} else if (type_name == Binding::double_name) {
			auto save = out.precision(numeric_limits<double>::max_digits10); //!! Did I leave max as a placeholder?
			auto val = *_PTR(double*);
			out << val;
			out.precision(save);
		} else if (type_name == Binding::string_name) {
			auto val = *_PTR(string*);
			out << val;
		} else if (type_name == Binding::char_name) { // Plain char value (not char*!)
			auto val = *_PTR(char*);
			out << val;
		} else {
			out << "ERROR: Can't show unknown type: " << d.type_name();
		}

	} catch(std::bad_any_cast&) {
cerr << "- ERROR: Type mismatch for a binding with saved type \"" <<type_name<< "\"!\n";
		// Nothing added to 'out', continuing...
	} catch(...) {
cerr << "- ERROR: Wow, unknown exception in " __FUNCTION__ "!\n";
	}
	
#undef _PTR

	return out;
}
