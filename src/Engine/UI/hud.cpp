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

#include "Engine/diag/Log.hpp"

using namespace std;
using namespace UI;

//----------------------------------------------------------------------------
void HUD::add(const char* literal)
{
//cerr << "---> HUD: ADDING const char* literal: "<<literal<<'\n';
	_elements.emplace_back(std::in_place_type<std::string>, literal);
}

void HUD::add(string literal)
{
//cerr << "---> HUD: ADDING std::string literal: "<<literal<<'\n';
	_elements.emplace_back(std::in_place_type<std::string>, literal);
}

void HUD::add(float literal)
{
//cerr << "---> HUD: ADDING float literal: "<<literal<<'\n';
	_elements.emplace_back(std::in_place_type<Binding>, literal);
}

void HUD::add(int literal)
{
//cerr << "---> HUD: ADDING float literal: "<<literal<<'\n';
	_elements.emplace_back(std::in_place_type<Binding>, literal);
}


void HUD::add(Binding::STRING_FN_PTR f)
{
//cerr <<"---> HUD: ADDING "<< Binding::fptr_name <<": "<< (void*)f <<" -> "<< f() <<'\n';
	_elements.emplace_back(f);
/*!! OLD:
	//-------------------------------------------------------------
		// Helpers to avoid including the monstrosity of <type_traits> just for std::remove_const:
		private: template <class T> struct _nonstd_remove_const          { typedef T type; };
		private: template <class T> struct _nonstd_remove_const<const T> { typedef T type; };
	public:
	template <typename T> auto add(T* var, const char* type_name)
	{
//!!?? [What did I mean below: shouldn't matter because I assumed they had the same size (they don't!),
//!!   or shouldn't matter for some other reason I failed to add?!... :-/ ]
//!!??Crashes if var is int*, as if sizeof int* < sizeof void* mattered, but it shouldn't:
//!!??		std::cerr << type_name << " -> " << (void*)any_cast<void*>(ptr) << " added." << endl;
//		std::cerr << type_name << " added." << endl;
	}
!!*/
}


//----------------------------------------------------------------------------
std::ostream& operator << (std::ostream& out, const UI::HUD& hud)
{
//cerr << "- HUD: serializing "<< hud.elements().size() <<" elements...\n";
	for (auto const& e : hud.elements()) {
		try {           out << any_cast<std::string>(e); continue;
		} catch (...) { //out << "[not a string ELEMENT]\n";
		}

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
