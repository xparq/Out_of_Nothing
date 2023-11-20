#ifndef _DMN78405B0T873YBV24C467I_
#define _DMN78405B0T873YBV24C467I_

#include <typeinfo>
#include <type_traits>
#include <any>
#include <functional>
#include <string>
#include <string_view>

namespace UI {

class Binding
{
protected:
	using _typename_t = decltype(typeid(void).name()); //!!?? Should just go const char* directly?

	std::any    _data_ptr;
	_typename_t _type;

public:
	constexpr auto&       ref() const { return _data_ptr; }
	constexpr auto& type_name() const { return _type; }

public:

	using STRING_F_PTR = std::string (*)(); // raw function ptr (see CALLBACK for functors/closures (capturing lambdas)!)

	using STRING_FUNCTOR = std::function<std::string()>; //! not a raw fn pointer, not a ptr at all, but a (stateful) function object, so not convertible to/from void*!
		//!NOTE: "stateless" lambdas will (or just could?) also get auto-converted to plain old functions!

	// Convenience ctors for string literals...
	Binding(const char* literal);
	Binding(const std::string* literal); //! Note the pointer! Will not accept "how_long_do_this_lives"s

	// Add any other pointer-type binding...
		// Helpers to avoid including the monstrosity of <type_traits> just for std::remove_const:
		private: template <class T> struct _nonstd_remove_const          { typedef T type; };
		private: template <class T> struct _nonstd_remove_const<const T> { typedef T type; };
	public:
	template <typename T> Binding(T* var, const char* type_name = nullptr) :
		_data_ptr(const_cast<typename _nonstd_remove_const<T>::type*>(var)),
		_type(type_name ? type_name : typeid(std::remove_const_t<T>).name())
	{}

	// This can't be part of the template, as stateless ("captureless") lambdas wouldn't match without casting!
	//!!?? [What did I mean? Lambdas with empty [] do match? :-o Is the standard?]
	Binding(STRING_F_PTR f);

	// Catch-all lambda matcher (needs no cast for lambdas, but we know kinda nothing here...)
	//!! IOW, this actually matches EVERYTHING, not just functors!
	template <typename ShouldBeFunctor> Binding(ShouldBeFunctor f) :
		_data_ptr((STRING_FUNCTOR)f),
		_type(functor_name)
	{
//!!?? Why is this never triggered:
		static_assert(!std::is_rvalue_reference_v<decltype(f)>, "Only lvalues are allowed for binding!");
//std::cerr << "- unknown type -- hopefully a lambda! :) -- catched...\n";
	}

	// "promptless watcher" call form (a bit too vague tho, but would "mostly work"...):
//	template <typename T> auto add(T* var) { return add("", var); }

// These are not matched for some reason. :-/ Why?
//friend	std::ostream& operator <<(std::ostream& out, const Binding& w);


//----------------------------------------------------------------------------
/*!! WIP !!
	template <typename T> auto get()
	{
		constexpr const std::string_view actual_type = type_name();
		constexpr const std::string_view requested_type = typeid(T).name();

	#define _PTR(type) ( any_cast<type>(ref()) ) // Throws bad_any_cast in case of a mismatch!

		try {
			//!! This is still too fragile! Remember: name() is implementation-dependent, and
			//!! could be anything, even duplicates(!)
			//!! E.g. with MSVC char* and char would both be just "char"! :-o

			if constexpr (requested_type == Binding::charptr_literal_name) {
				//!! Cf.: charptr_name!
				return _PTR(const char*);
			} else if constexpr (actual_type == Binding::charptr_name) { //!!??
	//cerr << "GOT 'charptr_name' ("<< type_name <<")\n";
		//!!WAS:	out << (const char*) (* _PTR(const char**)) << '\n';
		//!!??		out << _PTR(const char*) << '\n';
		//!!Why exactly does this need const?! Did crash with <char**>, omitting it -- was it just bac_any_cast?
		//!!		prompt = (const char*) (* _PTR(char**));
			} else if constexpr (requested_type == Binding::fptr_name) { // raw fn*
				return (Binding::STRING_F_PTR)_PTR(void*);
			} else if constexpr (requested_type == Binding::functor_name) { // CALLBACK functor
				return _PTR(Binding::STRING_FUNCTOR)();
			} else {
				return *_PTR(T*);
			}
		} catch(std::bad_any_cast&) {

	cerr << "- ERROR: " __FUNCTION__ ": Couldn't convert "<< actual_type <<" to "<< requested_type <<'\n';
			throw; // I'd prefer a safe ptr-to-empty return, but can't have a "universal" return type! :-/
		} catch(...) {
	cerr << "- ERROR: " __FUNCTION__ ": Wow, unknown exception!\n";
			throw; // I'd prefer a safe ptr-to-empty return, but can't have a "universal" return type! :-/
		}

	#undef _PTR
		[[unreachable]] // I'd prefer a safe ptr-to-empty return, but can't have a "universal" return type! :-/
		return nullptr;
	}
This not only doesn't compile, the compiler doesn't even survive attempting it!... :)

C:\sz\prj\Out_of_Nothing\build.out\v.crtdll.d\src\UI\Binding.hpp(131): fatal error C1001: Internal compiler error.
(compiler file 'D:\a\_work\1\s\src\vctools\Compiler\CxxFE\sl\p1\c\trees.c', line 8740)
To work around this problem, try simplifying or changing the program near the locations listed above.
If possible please provide a repro here: https://developercommunity.visualstudio.com
Please choose the Technical Support command on the Visual C++ Help menu, or open the Technical Support help file for more information
!!*/

	//----------------------------------------------------------------------------
	//!!
	//!! Dear C++... Why TF isn't typeid(...).name(), of all things, consteval?! :-o
	//!! (Also, not sure these could be auto, due to subtle const char* vs. char[] mismatches.)
	//!!
	// These types have their own add(), so we can "standardize" their names (if C++ had failed to do so):
	/*constexpr*/ static const _typename_t charptr_literal_name;// = "string literal"; //!!?? Why did this work (why different from const char*): typeid("literal").name()?
	/*constexpr*/ static const _typename_t fptr_name;//     = "fptr";     // synthetic name for directly (*f)() compatible raw fn ptrs
	/*constexpr*/ static const _typename_t functor_name;//  = "CALLBACK"; // synthetic name for <CALLBACK> functor objects
	// For the generic add():
	/*constexpr*/ static const _typename_t float_name;//   = typeid(float).name();
	/*constexpr*/ static const _typename_t double_name;//  = typeid(double).name();
	/*constexpr*/ static const _typename_t int_name;//     = typeid(int).name();
	/*constexpr*/ static const _typename_t bool_name;//    = typeid(bool).name();
	/*constexpr*/ static const _typename_t char_name;//    = typeid(char).name();
	/*constexpr*/ static const _typename_t charptr_name;// = typeid(const char*).name(); //!!??
	/*constexpr*/ static const _typename_t string_name;//  = typeid(std::string).name();

}; // class Binding

}; // namespace UI

//!!?? Declaring it as friend was not enough this time. But why?
std::ostream& operator <<(std::ostream& out, const UI::Binding& w);

#endif // _DMN78405B0T873YBV24C467I_
