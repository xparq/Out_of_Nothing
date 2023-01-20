#ifndef __HUD_SFML__
#define __HUD_SFML__

#include "cfg.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <vector>
	using std::vector;
#include <typeinfo>
#include <tuple>
	using std::tuple, std::make_tuple;
#include <any>
#include <functional> // function
#include <string> // getline
	using std::string;
#include <sstream> // istringstream, getline
#include <limits> // digit # for precise output
//#include <format>

using namespace std;

class HUD
{
	typedef tuple<string, std::any, string> ITEM; // type, var*, prompt
	typedef string (*CALLBACK)();

public:

/*!!
	template <typename T>
	auto add(std::function<T> f) {
	}
!!*/

	//!! add(prompt, string (*f)()) {...}
	void add(string (*f)()); // Alas, can't just do CALLBACK f... :-/
	void add(const char* literal);

	template <typename T>
	auto add(const char* prompt, T* var)
	{
		any ptr; ptr.emplace<T*>(var); //!!?? would work incorrectly (crash?!) with .emplace<void*>(var) -- but why?!
		datarefs.push_back(make_tuple(typeid(T).name(), ptr,
			prompt != "" ? string(prompt) + prompt_suffix : default_prompt));
//std::cerr << typeid(T).name() << " added.\n";
	}

	template <typename T>
	auto add(T* var) { return add("", var); }

	string render_to(std::stringstream& out);

	std::vector<ITEM> datarefs;

	string charptr_literal_name;
	string callable_name;
	string int_name;
	string float_name;
	string double_name;
	string char_name;
	string charptr_name;
	string string_name;

public:
	string default_prompt = "";
	string prompt_prefix =  "> ";
	string prompt_suffix =  ": ";

	HUD()
	{
		charptr_literal_name = "string literal"; //!!?? Why did this work (why different from const char*): typeid("literal").name()?
		callable_name        = typeid([]()->string{ return ""; }).name(); //!!?? "Callable"; //!

		int_name     = typeid(int).name();
		float_name   = typeid(float).name();
		double_name  = typeid(double).name();
		char_name    = typeid(char).name();
		charptr_name = typeid(const char*).name(); //!!??
		string_name  = typeid(string).name();
	}
};

//----------------------------------------------------------------------------
struct HUD_SFML : public HUD
{
//!!void draw(const Engine_SFML* engine);

	static constexpr auto CFG_HUD_FONT_PATH = "resource/fira.ttf";
	static constexpr uint32_t CFG_DEFAULT_TEXT_COLOR = 0x72c0c0ff; // RGBA

	void clear() { lines_to_draw.clear(); }
	auto line_count() const { return lines_to_draw.size(); }

	void append_line(const char* utf8_str)
	{
		lines_to_draw.emplace_back(utf8_str, font, cfg_hud_line_height);
		auto& line = lines_to_draw[line_count()-1];
		line.setPosition(sf::Vector2f(_panel_left, _panel_top + line_count() * cfg_hud_line_height));

//		line.setStyle(sf::Text::Bold | sf::Text::Underlined);
		line.setFillColor(sf::Color(CFG_DEFAULT_TEXT_COLOR));
	}

	void draw(sf::RenderWindow& window)
	{
		clear();

		std::stringstream ss; render_to(ss);
		for (std::string line; std::getline(ss, line);) {
			append_line(line.c_str());
		}		

		for (auto& text : lines_to_draw) {
			window.draw(text);
		}
	}

public:
	int cfg_hud_line_height = 13; //!!?? SFML docs say pixel, but what pixel?
	static int constexpr cfg_hud_panel_top = 0;
	static int constexpr cfg_hud_panel_left = -240;

	HUD_SFML(sf::RenderWindow& window, int xpos = cfg_hud_panel_left, int ypos = cfg_hud_panel_top) :
		_panel_top(ypos),
		_panel_left(xpos)
	{ _setup(window); }
	
	void _setup(sf::RenderWindow& window);

protected:
	std::vector<sf::Text> lines_to_draw;
	sf::Font font;

	int _panel_top;
	int _panel_left;
};

#endif // __HUD_SFML__
