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
	using std::cerr, std::endl;

using namespace std;
using namespace UI;

//----------------------------------------------------------------------------
void HUD::add(const char* literal)
{
//cerr << "---> HUD: ADDING literal: "<<literal<<"\n";
	prompts.emplace_back(""); // The binding will have the same string, and will << it too!
	watchers.emplace_back(literal);
}

void HUD::add(const string* literal)
{
//cerr << "---> HUD: ADDING literal: "<<literal<<"\n";
	prompts.emplace_back(""); // The binding will have the same string, and will << it too!
	watchers.emplace_back(literal);
}


void HUD::add(Binding::STRING_F_PTR f)
{
//std::cerr << "adding " << fptr_name << ": "
//		     << (void*)f << " -> " << f() << endl;
	return add("", (void*)f, Binding::fptr_name);
//!!??Alas, this crashes:
//!!??		return add("", f, fptr_name);

//		any ptr; ptr.emplace<void*>(f); //!!?? and why does <FPTR>(f) crash here??
//		watchers.push_back(make_tuple(fptr_name, ptr, default_prompt));
}

