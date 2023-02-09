#include "hud_sfml.hpp"

//#include <typeinfo>
#include <string>
	using std::string, std::getline;
#include <sstream>
	using std::stringstream;
#include <limits>
	using std::numeric_limits; // # of digits for precise float output
//#include <format>
#include <iostream>
	using std::cerr, std::endl;

using namespace std;

void HUD_SFML::_setup(sf::RenderWindow& window)
{
	if (!font.loadFromFile(CFG_HUD_FONT_PATH)) {
		// error...
	}

	// Adjust for negative "virtual" offsets:
	sf::Vector2u winsize = window.getSize();
	_panel_left = _panel_left < 0 ? winsize.x + _panel_left : _panel_left;
	//!!UNTESTED:
	_panel_top = _panel_top < 0 ? winsize.y + _panel_top : _panel_top;
}


//----------------------------------------------------------------------------
void HUD::add(const char* literal)
{
	any noptr; noptr.emplace<void*>(nullptr);
	watchers.push_back(make_tuple(charptr_literal_name, noptr, literal));
}

void HUD::add(FPTR f)
{
//std::cerr << "adding " << fptr_name << ": "
//		     << (void*)f << " -> " << f() << endl;
	return add("", (void*)f, fptr_name.c_str());
//!!??Alas, this crashes:
//!!??		return add("", f, fptr_name.c_str());

//		any ptr; ptr.emplace<void*>(f); //!!?? and why does <FPTR>(f) crash here??
//		watchers.push_back(make_tuple(fptr_name, ptr, default_prompt));
}

//----------------------------------------------------------------------------
string HUD::render_watched_item_to(std::stringstream& out)
{
	string result;
	for (auto ref : watchers) {

		//!! This is still too fragile! (Remember: name() is implementation-dependent, and
		//!! could be anything: mangled, duplicates, long raw template name for string etc.). 
		//!! E.g. with MSVC char* and char would both be just "char"! :-o

		// get<0>: type
		// get<1>: ref
		// get<2>: prompt

		if (string(get<0>(ref)) == charptr_literal_name) {
			out << get<2>(ref);
		} else if (string(get<0>(ref)) == fptr_name) { // raw fn*
			auto f = (FPTR)(any_cast<void*>(get<1>(ref)));
			out << get<2>(ref);
			out << f();
		} else if (string(get<0>(ref)) == functor_name) { // CALLBACK functor
			auto f = any_cast<CALLBACK>(get<1>(ref));
			out << get<2>(ref);
			out << f();
		} else if (string(get<0>(ref)) == int_name) {
			out << get<2>(ref);
			out << * any_cast<int*>(get<1>(ref));
		} else if (string(get<0>(ref)) == float_name) {
			auto save = out.precision(numeric_limits<float>::max_digits10);
			out << get<2>(ref);
			out << * any_cast<float*>(get<1>(ref));
			out.precision(save);
		} else if (string(get<0>(ref)) == double_name) {
			auto save = out.precision(numeric_limits<double>::max_digits10);
			out << get<2>(ref);
			out << * any_cast<double*>(get<1>(ref));
			out.precision(save);
		} else if (string(get<0>(ref)) == charptr_name) { // char*
			out << (const char*) (* any_cast<const char**>(get<1>(ref)));
//!!??Why would it crash with this, just omiting the const:			
//!!??				     << ", current val: " << (const char*) (* any_cast<char**>(get<1>(ref)));
		} else if (string(get<0>(ref)) == string_name) {
			out << get<2>(ref);
			out << * any_cast<string*>(get<1>(ref));
		} else if (string(get<0>(ref)) == char_name) { // char
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

//----------------------------------------------------------------------------
void HUD_SFML::draw(sf::RenderWindow& window)
{
	if (!active()) return;

	clear();

	std::stringstream ss; render_watched_item_to(ss);
	for (std::string line; std::getline(ss, line);) {
		append_line(line.c_str());
	}

	for (auto& text : lines_to_draw) {
		window.draw(text);
	}
}
