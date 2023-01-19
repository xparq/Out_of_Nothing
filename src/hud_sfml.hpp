#ifndef __HUD_SFML__
#define __HUD_SFML__

#include "cfg.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <vector>
#include <typeinfo>
#include <any>
#include <functional> // function
#include <string> // getline
#include <sstream> // istringstream, getline
#include <limits> // digit # for precise output
//#include <format>
#include <iostream>
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
	auto add(string (*f)()) { // Alas, can't just do CALLBACK f... :-/
	//!! Just forward to the generic add with no prompt!
cerr << callable_name << endl;
cerr << (void*)f << " -> " << f() << endl;

		any ptr; ptr.emplace<void*>(f); //!!?? why does <CALLBACK> crash here?!
		datarefs.push_back(make_tuple(callable_name, ptr, default_prompt));
	}

	auto add(const char* literal) {
		any noptr; noptr.emplace<void*>(nullptr);
		datarefs.push_back(make_tuple(charptr_literal_name, noptr, literal));
	}

	template <typename T>
	auto add(const char* prompt, T* var) {
		any ptr; ptr.emplace<T*>(var); //!!?? would work incorrectly (crash?!) with .emplace<void*>(var) -- but why?!
		datarefs.push_back(make_tuple(typeid(T).name(), ptr,
			prompt != "" ? string(prompt) + prompt_suffix : default_prompt));

cerr << typeid(T).name() << " added." << endl;
	}

	template <typename T>
	auto add(T* var) { return add("", var); }

	string render_to(std::stringstream& out)
	{
		string result;
		for (auto ref : datarefs) {

			//!! This is still too fragile! (Remember: name() is implementation-dependent, and
			//!! could be anything: mangled, duplicates, long raw template name for string etc.). 
			//!! E.g. with MSVC char* and char would both be just "char"! :-o

			if (string(get<0>(ref)) == charptr_literal_name) {
//				out << "[char* literal] ";
				out << get<2>(ref);
			} else if (string(get<0>(ref)) == callable_name) {
//				out << "[lambda] ";
				string (*f)() = (CALLBACK)(any_cast<void*>(get<1>(ref)));
				out << f();
			} else if (string(get<0>(ref)) == int_name) {
//				out << "[int] ";
				out << get<2>(ref);
				out << * any_cast<int*>(get<1>(ref));
			} else if (string(get<0>(ref)) == float_name) {
//				out << "[float] ";
				auto save = out.precision(numeric_limits<float>::max_digits10);
				out << get<2>(ref);
				out << * any_cast<float*>(get<1>(ref));
				out.precision(save);
			} else if (string(get<0>(ref)) == double_name) {
//				out << "[double] ";
				auto save = out.precision(numeric_limits<double>::max_digits10);
				out << get<2>(ref);
				out << * any_cast<double*>(get<1>(ref));
				out.precision(save);
			} else if (string(get<0>(ref)) == charptr_name) {
//				out << "[char*] ";
				out << (const char*) (* any_cast<const char**>(get<1>(ref)));
//!!??Why would it crash with this, just omiting the const:			
//!!??				     << ", current val: " << (const char*) (* any_cast<char**>(get<1>(ref)));
			} else if (string(get<0>(ref)) == string_name) {
//				out << "[string] ";
				out << get<2>(ref);
				out << * any_cast<string*>(get<1>(ref));
			} else if (string(get<0>(ref)) == char_name) {
//				out << "[char] ";
				out << get<2>(ref);
				out << * any_cast<char*>(get<1>(ref));

			} else {
				out << "Can't show unknown type: ";
				out << get<0>(ref);
			}
			out << "\n";
		}
		return out.str();
	}

	vector<ITEM> datarefs;

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
	int cfg_hud_panel_top = 0;
	int cfg_hud_panel_left = -240;

	HUD_SFML(sf::RenderWindow& window)
	{
		_setup(window);
	}
	
	void _setup(sf::RenderWindow& window)
	{
		if (!font.loadFromFile(CFG_HUD_FONT_PATH))
		{
			// error...
		}

		sf::Vector2u winsize = window.getSize();
		_panel_top = cfg_hud_panel_top;
		_panel_left = cfg_hud_panel_left < 0 ? winsize.x + cfg_hud_panel_left : cfg_hud_panel_left;
	}

protected:
	vector<sf::Text> lines_to_draw;
	sf::Font font;

	int _panel_top;
	int _panel_left;
};

#endif // __HUD_SFML__
