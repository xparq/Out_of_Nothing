#ifndef _XDJ8H37458B79V24657CVU8_
#define _XDJ8H37458B79V24657CVU8_

#include "Binding.hpp"

#include <string>
#include <utility> // std::exchange

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

public:

	//-------------------------------------------------------------
	// Add a string literal to the panel...
	void add(const char* literal);
	void add(const std::string* literal);

	//-------------------------------------------------------------
	// Add a string-returning fn. pointer...
	//!! Can't template this, as stateless (captureless) lambdas wouldn't match without casting!
	void add(Binding::STRING_F_PTR f);
	//!!auto add(prompt, STRING_F_PTR f) {...}

	//-------------------------------------------------------------
	// Add a lambda/closure/functor...
	// (Catch-all lambda matcher that needs no cast for lambdas, but we know nothing here...)
	//!! IOW, this actually matches EVERYTHING, not just functors!
	//!! Binding(T) is hoped to deal with that in a sensible way...
	template <typename ShouldBeFunctor> auto add(const char* prompt, ShouldBeFunctor f)
	{
		//!!?? Why is this never triggered:
		static_assert(!std::is_rvalue_reference_v<decltype(f)>, "Only lvalues are allowed for binding!");
//std::cerr << "- unknown type -- hopefully a lambda/functor! :) -- catched...\n";
		prompts.emplace_back(prompt && *prompt ? std::string(prompt): "");
		watchers.emplace_back(std::forward<ShouldBeFunctor>(f));
	}

	// "promptless watcher" form (scary vague, but mostly works...):
	//template <typename T> auto add(T* var) { return add("", var); }

	//-------------------------------------------------------------
	// Add any of tha above, with a prompt...

		// Helpers to avoid including the monstrosity of <type_traits> just for std::remove_const:
		private: template <class T> struct _nonstd_remove_const          { typedef T type; };
		private: template <class T> struct _nonstd_remove_const<const T> { typedef T type; };
	public:
	template <typename T> auto add(const char* prompt, T* var, const char* type_name = nullptr)
	{
		prompts.emplace_back(prompt && *prompt ? std::string(prompt) : "");
		watchers.emplace_back(var, type_name);
//!!?? [What did I mean below: shouldn't matter because I assumed they had the same size (they don't!),
//!!   or shouldn't matter for some other reason I failed to add?!... :-/ ]
//!!??Crashes if var is int*, as if sizeof int* < sizeof void* mattered, but it shouldn't:
//!!??		std::cerr << type_name << " -> " << (void*)any_cast<void*>(ptr) << " added." << endl;
//		std::cerr << type_name << " added." << endl;
	}

protected:
	std::vector<Binding> watchers;
	std::vector<std::string> prompts;

	bool _active = true;

public:
	bool active() const       { return _active; }
	bool active(bool active)  { return std::exchange(_active, active); }

	//virtual void onResize(size_t width, size_t height) {}
	virtual ~HUD() = default;

}; // class HUD

}; // namespace UI

#endif // _XDJ8H37458B79V24657CVU8_
