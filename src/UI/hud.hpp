#ifndef _XDJ8H37458B79V24657CVU8_
#define _XDJ8H37458B79V24657CVU8_

#include "Binding.hpp"

#include <string>
	using namespace std::string_literals;
#include <string_view>
#include <any>
#include <utility> // std::exchange
#include <ostream>

#ifdef DEBUG
#  include <iostream> // for debugging only!
	using std::cerr;
#  include <type_traits> // is_same
#endif

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
	void add(std::string literal);
	void add(float literal);
	void add(int literal);

	//-------------------------------------------------------------
	// Add a string-returning fn. pointer...
	//!! Can't template this, as stateless (captureless) lambdas wouldn't match without casting!
	void add(Binding::STRING_FN_PTR f);

	//-------------------------------------------------------------
	// Scary vague version, but mostly works...
	template <typename T> auto add(T* var)
	{
		//!!?? Why is this never triggered:
//!!		static_assert(!std::is_rvalue_reference_v<decltype(T)>, "Only lvalues are allowed for binding!");
//std::cerr << "- unknown type -- hopefully a lambda/functor! :) -- catched...\n";
		_elements.emplace_back(std::in_place_type<Binding>, std::forward<T*>(var));
	}

	//-------------------------------------------------------------
	// Even more vague, but mostly still works! :)
	template <typename ShouldBeFunctor> auto add(ShouldBeFunctor f)
	{
		//!!?? Why is this never triggered:
		static_assert(!std::is_rvalue_reference_v<decltype(f)>, "Only lvalues are allowed for binding!");
//std::cerr << "- unknown type -- hopefully a lambda/functor! :) -- catched...\n";
		_elements.emplace_back(std::in_place_type<Binding>, std::forward<ShouldBeFunctor>(f));
	}


	template <typename T>
	auto& operator << (T* var) {
		add(std::forward<T*>(var));
#ifdef DEBUG
		if constexpr (std::is_same<std::decay_t<T>, char>::value) {
//cerr << "- HUD<< added (const?) char* element \""<< var <<"\" (elements: "<<_elements.size()<<").\n";
		} else {
//cerr << "- HUD<< added pointer -- hopefully the addr. of a variable! :) (elements: "<<_elements.size()<<").\n";
		}
#endif
		return *this;
	}

	template <typename T>
	auto& operator << (T&& var) {
		add(std::forward<T&&>(var));
#ifdef DEBUG
//cerr << "- HUD<< added element of unknown type -- hopefully a Functor! :) (elements: "<<_elements.size()<<").\n";
#endif
		return *this;
	}

protected:
	std::vector<std::any> _elements; //!! std::vector<Element>... <- Couldn't sensibly wrap the raw `any` yet.
	bool _volatile = true; // Assume yes, until known to not be
	bool _active = true;

public:
	bool active() const       { return _active; }
	bool active(bool active)  { return std::exchange(_active, active); }

	HUD() = default;
	HUD(const HUD&) = delete;
	HUD(HUD&&) = delete;
	virtual ~HUD() = default;

//!!??friend std::ostream& operator << (std::ostream& out, const UI::HUD& hud);
	const std::vector<std::any>& elements() const { return _elements; }
}; // class HUD

}; // namespace UI

//!!?? Why TF isn't the friend decl. working, again (see Binding, too...)?!
std::ostream& operator << (std::ostream& out, const UI::HUD& hud);

#endif // _XDJ8H37458B79V24657CVU8_
