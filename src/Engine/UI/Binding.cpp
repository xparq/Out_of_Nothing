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
#include <iomanip>
	using std::setprecision;

#include "Engine/diag/Error.hpp"
//#include "Engine/diag/Log.hpp"

using namespace std;
using namespace UI;

//!! Should be moved further out into some separate cpp_type_bullshit_..._something object instead!
//!! Also, not sure these could be auto, due to subtle const char* vs. char[...] mismatches...
// These have their own add(...):
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::charptr_literal_name = "char* literal";
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::string_literal_name  = "string literal";
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::int_literal_name     = "int literal";
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::float_literal_name   = "float literal";
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::double_literal_name  = "double literal";

/*static*/ /*constexpr*/ const Binding::_typename_t Binding::charptr_fn_ptr_name  = "charptr_fn_ptr";  // synthetic name for const char* (*f)() and compatible raw fn ptrs
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::string_fn_ptr_name   = "string_fn_ptr";   // synthetic name for directly (*f)() compatible raw fn ptrs
/*static*/ /*constexpr*/ const Binding::_typename_t Binding::string_functor_name  = "string_closure";  // synthetic name for <CALLBACK> functor Binding::objects

// :-( /*static*/ /*constexpr*/ const Binding::_typename_t Binding::sg_printable_name    = "something_<<able";

//!!
//!! It's totally blurred, where exactly the distinction between pointers and values are made!
//!! These are non-pointer types, but the whole thing only supports pointers, not values! :-o
//!! So: op<< assumes these plain names actually mean pointers, and casts accordingly! :-o
//!!
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
	charptr_fn_ptr_name  = "charptr_fn_ptr"; // synthetic name for const char* (*f)() and compatible raw fn ptrs
	string_fn_ptr_name   = "string_fn_ptr";  // synthetic name for directly (*f)() compatible raw fn ptrs
	string_functor_name  = "string_closure"; // synthetic name for <STRING_FUNCTOR> objects

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
	_data_ptr(literal), _type(charptr_literal_name)
{
//cerr << "---> Binding: ADDING const char* literal: "<<literal<<"\n";
}

Binding::Binding(Binding::STRING_FN_PTR f) :
//	Binding((void*)f, string_fn_ptr_name)
	_data_ptr(f), _type(string_fn_ptr_name)
{
//cerr << "---> Binding: ADDING " << string_fn_ptr_name << ": " << (void*)f << " -> " << f() << "\n";
}

Binding::Binding(CHARPTR_FN_PTR f) :
	_data_ptr(f), _type(charptr_fn_ptr_name)
{
//cerr << "---> Binding: ADDING " << charptr_fn_ptr_name << ": " << (void*)f << " -> " << f() << "\n";
}

//----------------------------------------------------------------------------
std::ostream& operator <<(std::ostream& out, const UI::Binding& d)
{
	const string_view type_name = d.type_name();

//cerr << "static Binding::charptr_name:         " << Binding::charptr_name << "         --- retrieved type_name: " << type_name << "\n";
//cerr << "static Binding::charptr_literal_name: " << Binding::charptr_literal_name << " --- retrieved type_name: " << type_name << "\n";
//cerr << "static Binding::float_name:           " << Binding::float_name << "           --- retrieved type_name: " << type_name << "\n";

#define _CAST(type) ( any_cast<type>(d.ref()) )

	//!!MOVE THIS ENTIRE CAST-BACK PART TO A TEMPLATED AUTO-CONVERSION MEMBER? (IF THAT'S POSSIBLE AT ALL)
	try {
		//!! This is still too fragile! Remember: name() is implementation-dependent, and
		//!! could be anything: duplicates(!), mangled, long raw template name (even for string) etc.
		//!! E.g. with MSVC char* and char would both be just "char"! :-o

		if (type_name == Binding::charptr_literal_name) {
			//!! Cf.: charptr_name!
			auto val = _CAST(const char*);
			out << val;
		} else if (type_name == Binding::charptr_name) { //!!??
//cerr << "GOT 'charptr_name' ("<< type_name <<")\n";
	//!!WAS:	out << (const char*) (* _CAST(const char**)) << '\n';
	//!!??		out << _CAST(const char*) << '\n';
	//!!Why exactly does this need const?! Did crash with <char**>, omitting it -- was it just bac_any_cast?
	//!!		prompt = (const char*) (* _CAST(char**));
		} else if (type_name == Binding::string_fn_ptr_name) { // raw fn*
			out << _CAST(Binding::STRING_FN_PTR) ();
		} else if (type_name == Binding::charptr_fn_ptr_name) { // raw fn*
			out << _CAST(Binding::CHARPTR_FN_PTR) ();
		} else if (type_name == Binding::string_functor_name) {
			out << _CAST(Binding::STRING_FUNCTOR) ();
		} else if (type_name == Binding::int_name) {
			out << *_CAST(int*);
		} else if (type_name == Binding::bool_name) {
			out << (*_CAST(bool*) ? "on" : "off"); //!! This should be configurable, and also to be used by bool literals, if implemented!
		} else if (type_name == Binding::float_name) {
			auto save = out.precision(numeric_limits<float>::max_digits10); //!! Did I use max as a placeholder?
			out << *_CAST(float*);
			out.precision(save);
		} else if (type_name == Binding::double_name) {
			auto save = out.precision(numeric_limits<double>::max_digits10); //!! Did I use max as a placeholder?
			out << *_CAST(double*);
			out.precision(save);
		} else if (type_name == Binding::string_name) {
			out << *_CAST(string*);
		} else if (type_name == Binding::char_name) { // Plain char value (not char*!)
			out << *_CAST(char*);
		} else
		// Literals...
		       if (type_name == Binding::string_literal_name) {
			out << _CAST(const string&);
		} else if (type_name == Binding::float_literal_name) {
			out << _CAST(float);
		} else if (type_name == Binding::int_literal_name) {
			out << _CAST(int);
		} else if (type_name == Binding::double_literal_name) {
			out << _CAST(double);
		} else {
#ifdef DEBUG
			out << "Binding ERROR: Can't show unknown type: " << d.type_name();
#endif
		}

	} catch(std::bad_any_cast&) {
		ERROR("Type mismatch binding with saved type \"" + string(type_name) + "\" [" + Binding::string_fn_ptr_name + "]!");
		// Nothing added to 'out', continuing...
	} catch(...) {
		ERROR("Wow, unknown exception in " + __FUNCTION__ + "!");
	}
	
#undef _CAST

	return out;
}
