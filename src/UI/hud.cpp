#include "hud.hpp"

//#include <typeinfo>
#include <string>
	using std::string, std::getline;
#include <string_view>
	using std::string_view;
#include <sstream>
	using std::stringstream;
#include <limits>
	using std::numeric_limits; // # of digits for precise float output
//#include <format>
#include <iostream>
	using std::cerr, std::endl; // Only for debugging!

using namespace std;
using namespace UI;

//----------------------------------------------------------------------------
void HUD::add(const char* literal)
{
//cerr << "---> HUD: ADDING const char* literal: "<<literal<<'\n';
	_elements.emplace_back(std::in_place_type<std::string>, literal);
}

void HUD::add(const string* literal)
{
//cerr << "---> HUD: ADDING std::string literal: "<<literal<<'\n';
	_elements.emplace_back(std::in_place_type<std::string>, *literal);
}

void HUD::add(Binding::STRING_F_PTR f)
{
//cerr <<"---> HUD: ADDING "<< Binding::fptr_name <<": "<< (void*)f <<" -> "<< f() <<'\n';
	return add("", f, Binding::fptr_name);
}

//----------------------------------------------------------------------------
std::ostream& operator << (std::ostream& out, const UI::HUD& hud)
{
//cerr << "- HUD: serializing "<< hud.elements().size() <<" elements...\n";
	for (auto const& e : hud.elements()) {
		try {           out << any_cast<std::string>(e); continue;
		} catch (...) { //out << "[not a string ELEMENT]\n";
		}

		try {           out << any_cast<const char*>(e); continue;
		} catch (...) { //out << "[not a const char* ELEMENT]\n";
		}

		try {           out << any_cast<char*>(e); continue;
		} catch (...) { //out << "[not a char* ELEMENT]\n";
		}

		try {           out << any_cast<Binding>(e); continue;
		} catch (...) { //out << "[not a binding ELEMENT]\n";
		}
	}
	return out;
}
