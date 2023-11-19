#ifndef _DMN78405B0T873YBV24C467I_
#define _DMN78405B0T873YBV24C467I_

#include <typeinfo>
#include <tuple>
	//using std::tuple, std::make_tuple;
#include <any>
	//using std::any;
#include <functional>
	//using std::function;
#include <string>
	//using std::string;

namespace UI {

class Binding
{
public://!!
	using _nametype_ = decltype(typeid(void).name());

protected:
	using WATCHER = std::tuple<_nametype_    // type name
				 , std::any>;    // pointer to data, or free function ptr, or CALLBACK _object_

	WATCHER _binding;

public:
	_nametype_  type_name() const { return std::get<0>(_binding); }
	auto        ref()       const { return std::get<1>(_binding); }

public:
	using FPTR = std::string (*)(); // raw function ptr (see CALLBACK for functors/closures (capturing lambdas)!)

	//!NOTE: "stateless" lambdas will (or just could?) also get auto-converted to plain old functions!
	using CALLBACK = std::function<std::string()>; //! not a raw fn pointer, not a ptr at all, but a (stateful) function object, so not convertible to/from void*!

public:
	Binding(const char* literal);
//!!	Binding(string literal);

	// Add a var. binding (+ a prompt)
	// Helpers to avoid #include <type_traits> for std::remove_const...:
	private: template <class T> struct _nonstd_remove_const          { typedef T type; };
	private: template <class T> struct _nonstd_remove_const<const T> { typedef T type; };
	public:
	template <typename T> Binding(T* var, const char* type_name = nullptr)
	{
		_binding = make_tuple(  type_name ? type_name : typeid(std::remove_const_t<T>).name()
		                   , std::any(//           typename _nonstd_remove_const<T>::type*(
		                              const_cast<typename _nonstd_remove_const<T>::type*>(var)
					      //)
				             )
		                  )
		;
			//!!?? would work incorrectly (crash?!) with .emplace<void*>(var) -- but why?!
//!!?? [What did I mean below: "shouldn't matter" because I assumed they had the same size (they don't!),
//!!   or "shouldn't matter" for some other reason I failed to add?!... :-/ ]
//!!??Crashes if var is int*, as if sizeof int* < sizeof void* mattered, but it shouldn't:
//!!??		std::cerr << type_name << " -> " << (void*)any_cast<void*>(ptr) << " added." << endl;
//		std::cerr << type_name << " added." << endl;
	}

	// This can't be part of the template, as stateless ("captureless") lambdas wouldn't match without casting!
	//!!?? [Again: What did I mean? Lambdas with empty [] do match? :-o ]
	Binding(FPTR f);

	// Catch-all lambda matcher (needs no cast for lambdas, but we know kinda nothing here...)
	template <typename F> Binding(F f)
	{
//std::cerr << "- unknown lambda catched...\n";
		std::any functor; functor.emplace<CALLBACK>((CALLBACK)f);
		_binding = make_tuple(functor_name, functor);
	}

	// "promptless watcher" call form (a bit too vague tho, but would "mostly work"...):
//	template <typename T> auto add(T* var) { return add("", var); }

// These are not matched for some reason. :-/ Why?
//friend	std::ostream& operator <<(std::ostream& out, const Binding& w);

	//!! FFS, thanks again, C++... If anything, this really should've been constexpr, but isn't:
	//!! Also, not sure these could be auto, due to subtle const char* vs. char[...] mismatches...
	// These types have their own add(), so we can "standardize" their names (if the C++ committee failed to do so ;) ):
	static const _nametype_ charptr_literal_name;// = "string literal"; //!!?? Why did this work (why different from const char*): typeid("literal").name()?
	static const _nametype_ fptr_name;//     = "fptr";     // synthetic name for directly (*f)() compatible raw fn ptrs
	static const _nametype_ functor_name;//  = "CALLBACK"; // synthetic name for <CALLBACK> functor objects
	// For the generic add():
	static const _nametype_ float_name;//   = typeid(float).name();
	static const _nametype_ double_name;//  = typeid(double).name();
	static const _nametype_ int_name;//     = typeid(int).name();
	static const _nametype_ bool_name;//    = typeid(bool).name();
	static const _nametype_ char_name;//    = typeid(char).name();
	static const _nametype_ charptr_name;// = typeid(const char*).name(); //!!??
	static const _nametype_ string_name;//  = typeid(std::string).name();

}; // class Binding

}; // namespace UI

//!!?? Declaring it as friend was not enough this time. But why?
std::ostream& operator <<(std::ostream& out, const UI::Binding& w);

#endif // _DMN78405B0T873YBV24C467I_
