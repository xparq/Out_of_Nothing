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


//!! add(prompt, string (*f)()) {...}
void HUD::add(string (*f)())
{
//!! Just forward to the generic add with no prompt!
cerr << callable_name << endl;
cerr << (void*)f << " -> " << f() << endl;

	any ptr; ptr.emplace<void*>(f); //!!?? why does <CALLBACK> crash here?!
	datarefs.push_back(make_tuple(callable_name, ptr, default_prompt));
}

void HUD::add(const char* literal)
{
	any noptr; noptr.emplace<void*>(nullptr);
	datarefs.push_back(make_tuple(charptr_literal_name, noptr, literal));
}


string HUD::render_to(std::stringstream& out)
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

