#ifndef _XDJ8H37458B79V24657CVU8_
#define _XDJ8H37458B79V24657CVU8_

#include <vector>
	//using std::vector;
#include <typeinfo>
#include <tuple>
	//using std::tuple, std::make_tuple;
#include <any>
	//using std::any;
#include <functional>
	//using std::function;
#include <string>
	//using std::string;
//!!#include <string_view>
//!!	//using std::string_view;
#include <utility>
	//using std::exchange;


namespace UI {

class HUD
{
public:
	// Note: all in pixels
	static constexpr int DEFAULT_LINE_HEIGHT = 13; //! Lots of obscure warnings if this was size_t! :-o
	static constexpr int DEFAULT_PANEL_TOP = 4; // signed!
	static constexpr int DEFAULT_PANEL_LEFT = -250; // signed!
	static constexpr size_t DEFAULT_PANEL_WIDTH = 0; //!! 0: fit-text - NOT IMPLEMENTED YET!
	static constexpr size_t DEFAULT_PANEL_HEIGHT = 0; //!! 0: fit-text - NOT IMPLEMENTED YET!
	static constexpr int DEFAULT_PADDING = 6;

protected: using _nametype_ = decltype(typeid(void).name());

public: // Failed to declare the friend << functions (they're just not matched...), so here we go... :-/
	using FPTR = std::string (*)(); // raw function ptr (see CALLBACK for functors/closures (capturing lambdas)!)

	//!NOTE: "stateless" lambdas will (or just could?) also get auto-converted to plain old functions!
	using CALLBACK = std::function<std::string()>; //! not a raw fn pointer, not a ptr at all, but a (stateful) function object, so not convertible to/from void*!

	using WATCHER = std::tuple<_nametype_ // type name
	                         , std::any   // pointer to data, or free function ptr, or CALLBACK _object_
                                 , std::string>;   // prompt

public:
	void add(const char* literal); // This is the only literal supported yet!

	// Add a var. binding (+ a prompt)
	// Helpers to avoid #include <type_traits> for std::remove_const...:
	private: template <class T> struct _nonstd_remove_const          { typedef T type; };
	private: template <class T> struct _nonstd_remove_const<const T> { typedef T type; };
	public:
	template <typename T> auto add(const char* prompt, T* var, const char* type_name = nullptr)
	{
		if (!type_name) type_name = typeid(std::remove_const_t<T>).name();
		std::any ptr;
		         ptr.emplace<typename _nonstd_remove_const<T>::type*>(
		          const_cast<typename _nonstd_remove_const<T>::type*>(var));
			//!!?? would work incorrectly (crash?!) with .emplace<void*>(var) -- but why?!
		watchers.push_back(make_tuple(type_name, ptr,
		                              prompt && *prompt ? std::string(prompt) + prompt_suffix : default_prompt));
//!!?? [What did I mean below: shouldn't matter because I assumed they had the same size (they don't!),
//!!   or shouldn't matter for some other reason I failed to add?!... :-/ ]
//!!??Crashes if var is int*, as if sizeof int* < sizeof void* mattered, but it shouldn't:
//!!??		std::cerr << type_name << " -> " << (void*)any_cast<void*>(ptr) << " added." << endl;
//		std::cerr << type_name << " added." << endl;
	}

	//!!auto add(prompt, FPTR f) {...}

	// Can't template this, as stateless ("captureless") lambdas wouldn't match without casting!
	void add(FPTR f);

	// Catch-all lambda matcher (needs no cast for lambdas, but we know kinda nothing here...)
	template <typename F> auto add(const char* prompt, F f)
	{
//std::cerr << "- unknown lambda catched...\n";
		std::any functor; functor.emplace<CALLBACK>((CALLBACK)f);
		watchers.push_back(make_tuple(functor_name, functor,
		                              prompt && *prompt ? std::string(prompt) + prompt_suffix : default_prompt));
	}

	// "promptless watcher" call form (a bit too vague tho, but mostly works):
//	template <typename T> auto add(T* var) { return add("", var); }

// These are not matched for some reason. :-/ Why?
friend	std::ostream& operator <<(std::ostream& out, const WATCHER& w);

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

protected:
	std::vector<WATCHER> watchers;

	std::string default_prompt = "";
	std::string prompt_prefix =  "> ";
	std::string prompt_suffix =  ": ";

	bool _active = true;

public:
	// Must be implemented by derived classes:
	//static HUD* create() { static_assert(false, "::create() should be supplied by implementation classes!"); }

	void _static_init();
	HUD() { _static_init(); }

	bool active() const       { return _active; }
	bool active(bool active)  { return std::exchange(_active, active); }

	//virtual void onResize(size_t width, size_t height) {}
	virtual ~HUD() = default;

}; // class HUD

}; // namespace UI

//!! Declaring it friend was not enough this time. Why? Sigh...
std::ostream& operator <<(std::ostream& out, const UI::HUD::WATCHER& w);

#endif // _XDJ8H37458B79V24657CVU8_
