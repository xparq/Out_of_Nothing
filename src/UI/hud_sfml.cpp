#include "hud_sfml.hpp"

#include <SFML/Graphics/RectangleShape.hpp>

//!!Struggling with doing a cliprect... (-> draw())
//!!#include <SFML/Graphics/View.hpp>
//!!#include "TGUI-Clipping.hpp"

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

namespace UI {

//----------------------------------------------------------------------------
void HUD_SFML::_setup(sf::RenderWindow& window)
{
	if (!font.loadFromFile(CFG_HUD_FONT_PATH)) {
		//! SFML does print errors to the console.
		active(false);
	}

	// Adjust for negative "virtual" offsets:
	sf::Vector2u winsize = window.getSize();

	_panel_left = req_panel_left < 0 ? winsize.x + req_panel_left : req_panel_left;
	_panel_top  = req_panel_top  < 0 ? winsize.y + req_panel_top  : req_panel_top;
}

//----------------------------------------------------------------------------
void HUD_SFML::onResize(sf::RenderWindow& window)
{
	_setup(window);
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
		} else if (string(get<0>(ref)) == bool_name) {
			out << get<2>(ref);
			out << * any_cast<bool*>(get<1>(ref));
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
void HUD_SFML::append_line(const char* utf8_str)
{
	lines_to_draw.emplace_back(utf8_str, font, DEFAULT_LINE_HEIGHT);
	auto& line = lines_to_draw[line_count()-1];
	line.setPosition({
			(float)_panel_left + DEFAULT_PADDING,
			(float)_panel_top  + DEFAULT_PADDING + (line_count()-1) * DEFAULT_LINE_HEIGHT});

//		line.setStyle(sf::Text::Bold | sf::Text::Underlined);
	line.setFillColor(sf::Color(_fgcolor));
}

//----------------------------------------------------------------------------
void HUD_SFML::draw(sf::RenderWindow& window)
{
	if (!active()) return;

    //https://en.sfml-dev.org/forums/index.php?topic=25552.0
//!!Why the offset?!
//!!	tgui::Clipping clipview(window, sf::RenderStates::Default, //!!??
//!!	                  {(float)_panel_left, (float)_panel_top}, {200.f, 200.f});

//	auto saved_view = window.getView();
/*!!EMPTY... :-/ :
	sf::View vw;//({(float)_panel_left, (float)_panel_top}, {1200.f, 1200.f});
	vw.setViewport(sf::FloatRect({(float)_panel_left, (float)_panel_top}, {1.f, 1.f}));
	window.setView(vw);
!!*/
	clear_content();
	std::stringstream ss; render_watched_item_to(ss);
	for (std::string line; std::getline(ss, line);) {
		append_line(line.c_str());
	}

	// OK, finally draw something:
	sf::RectangleShape rect({450, //!!... "Fit-to-text" feature by recompilation ;)
		(float)lines_to_draw.size() * DEFAULT_LINE_HEIGHT + 2*DEFAULT_PADDING});//!! 0 for now: {(float)_panel_width, (float)_panel_height)};
	rect.setPosition({(float)_panel_left, (float)_panel_top});
	rect.setFillColor(sf::Color(_bgcolor));
	rect.setOutlineColor(sf::Color((uint32_t)(_bgcolor * 1.5f)));
	rect.setOutlineThickness(1);
	window.draw(rect);

	for (auto& text : lines_to_draw) {
		window.draw(text);
	}

//	window.setView(saved_view);
}

}; // namespace