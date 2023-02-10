#ifndef __HUD_SFML__
#define __HUD_SFML__

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

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
	using std::string;
# include <utility>
	//using std::exchange;

class HUD
{
//public: // <- Questionable... Just a convenience typedef for very little gain:
protected:
	using FPTR = string (*)(); // raw function ptr (see CALLBACK for functors/closures (capturing lambdas)!)
protected:
	//!NOTE: "stateless" lambdas will (or just could?) also get auto-converted to plain old functions!
	using CALLBACK = std::function<string()>; //! not a raw fn pointer, not a ptr at all, but a (stateful) function object, so not convertible to/from void*!
	using WATCHER = std::tuple<string // typename
	                                 , std::any // src* (data address or free function ptr), or CALLBACK _object_
                                     , string>; // prompt

public:

	void add(const char* literal);

	template <typename T> auto add(const char* prompt, T* var, const char* _tyepname = nullptr)
	{
		if (!_tyepname) _tyepname = typeid(T).name();
		std::any ptr; ptr.emplace<T*>(var); //!!?? would work incorrectly (crash?!) with .emplace<void*>(var) -- but why?!
		watchers.push_back(make_tuple(
			_tyepname,
			ptr,
			prompt && *prompt ? string(prompt) + prompt_suffix : default_prompt));
//!!??Crashes if var is int*, as if sizeof int* < sizeof void* mattered, but it shouldn't:
//!!??		std::cerr << _tyepname << " -> " << (void*)any_cast<void*>(ptr) << " added." << endl;
//		std::cerr << _tyepname << " added." << endl;
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
							prompt && *prompt ? string(prompt) + prompt_suffix : default_prompt));
	}

	// "promptless watcher" call form (a bit too vague tho, but mostly works):
//	template <typename T> auto add(T* var) { return add("", var); }

	string render_watched_item_to(std::stringstream& out);

	std::vector<WATCHER> watchers;

	string charptr_literal_name;
	string fptr_name;
	string functor_name;
	string int_name;
	string bool_name;
	string float_name;
	string double_name;
	string char_name;
	string charptr_name;
	string string_name;

protected:
	string default_prompt = "";
	string prompt_prefix =  "> ";
	string prompt_suffix =  ": ";

	bool _active = true;

public:
	HUD()
	{
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

	bool active() const       { return _active; }
	bool active(bool active)  { return std::exchange(_active, active); }
};

//----------------------------------------------------------------------------
struct HUD_SFML : public HUD
{
//!!void draw(const Engine_SFML* engine);

	static constexpr auto CFG_HUD_FONT_PATH = "asset/font/HUD.font";
	static constexpr uint32_t CFG_DEFAULT_TEXT_COLOR = 0x72c0c0ff; // RGBA
	static constexpr uint32_t CFG_DEFAULT_BACKGROUND_COLOR = 0x00406050;

	// All these are in pixels:
	static constexpr int CFG_HUD_LINE_HEIGHT = 13; //! Lots of obscure warnings if this was size_t! :-o
	static constexpr int CFG_HUD_PANEL_TOP = 4; // signed!
	static constexpr int CFG_HUD_PANEL_LEFT = -240; // signed!
	static constexpr size_t CFG_HUD_PANEL_WIDTH = 0; //!! 0: fit-text
	static constexpr size_t CFG_HUD_PANEL_HEIGHT = 0; //!! 0: fit-text
	static constexpr int CFG_HUD_PADDING = 6;

	void clear_content() { lines_to_draw.clear(); }
	auto line_count() const { return lines_to_draw.size(); }
	void append_line(const char* utf8_str);
	void draw(sf::RenderWindow& window);
	void _setup(sf::RenderWindow& window);

public:
	HUD_SFML(sf::RenderWindow& window, int xpos = CFG_HUD_PANEL_LEFT, int ypos = CFG_HUD_PANEL_TOP,
		uint32_t fgcolor = CFG_DEFAULT_TEXT_COLOR, uint32_t bgcolor = CFG_DEFAULT_BACKGROUND_COLOR)
		:
		_panel_top(ypos),
		_panel_left(xpos),
		_fgcolor(fgcolor),
		_bgcolor(bgcolor)
	{ _setup(window); }

protected:
	std::vector<sf::Text> lines_to_draw;
	sf::Font font;

	int _panel_top;
	int _panel_left;
	size_t _panel_width = CFG_HUD_PANEL_WIDTH;
	size_t _panel_height = CFG_HUD_PANEL_HEIGHT;

	uint32_t _fgcolor = CFG_DEFAULT_TEXT_COLOR;
	uint32_t _bgcolor = CFG_DEFAULT_BACKGROUND_COLOR;
};

#endif // __HUD_SFML__
