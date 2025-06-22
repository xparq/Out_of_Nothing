#ifndef _XDJ8H37458B79V24657CVU8_
#define _XDJ8H37458B79V24657CVU8_

#include "Binding.hpp"

#include <string>
	using namespace std::string_literals;
#include <string_view>
#include <cstdint>
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
	static constexpr int DEFAULT_LINE_HEIGHT = 17;
	static constexpr int DEFAULT_LINE_SPACING = 4;  // font size = line height - line spacing
	static constexpr int DEFAULT_PANEL_TOP = 4;     // <0: relative to top edge
	static constexpr int DEFAULT_PANEL_LEFT = -250; // <0: relative to right edge
	static constexpr unsigned DEFAULT_PANEL_WIDTH = 0;  // 0: fit text
	static constexpr unsigned DEFAULT_PANEL_HEIGHT = 0; // 0: fit text (!!fixed height not implemented!!)
	static constexpr int DEFAULT_PADDING = 4;

	static constexpr uint32_t DEFAULT_TEXT_COLOR = 0x72c0c0ff; // RGBA
	static constexpr uint32_t DEFAULT_BACKGROUND_COLOR = 0x00406050;

	struct Config
	{
		std::string font_file;
		unsigned line_height = DEFAULT_LINE_HEIGHT;
		unsigned line_spacing = 2;

		int panel_left   = DEFAULT_PANEL_LEFT;
		int panel_top    = DEFAULT_PANEL_TOP;
		int panel_width  = DEFAULT_PANEL_WIDTH;
		int panel_height = DEFAULT_PANEL_HEIGHT;

		uint32_t fgcolor = DEFAULT_TEXT_COLOR;
		uint32_t bgcolor = DEFAULT_BACKGROUND_COLOR;
	};

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
	bool _volatile = true; // Always needs refreshing before rendering? Assume yes, until known to not be.
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


//----------------------------------------------------------------------------
// Geometry...
//----------------------------------------------------------------------------
//!!public:
//!!	void resize(unsigned width, unsigned height);

//----------------------------------------------------------------------------
// Overridables...
//----------------------------------------------------------------------------
protected:
public: //!! Kludge to spare another public API like resize(), as this is gonna
        //!! become an SFW widget later anyway, with its API revamped!...
	virtual void onResize(unsigned /*width*/, unsigned /*height*/) {}

}; // class HUD

}; // namespace UI

//!!?? Why TF isn't the friend decl. working, again (see Binding, too...)?!
std::ostream& operator << (std::ostream& out, const UI::HUD& hud);

#endif // _XDJ8H37458B79V24657CVU8_
