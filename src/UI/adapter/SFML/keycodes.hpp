#ifndef _UI_SFML_keycodes_hpp_
#define _UI_SFML_keycodes_hpp_

#include "UI/Input.hpp"
#include "SFML/Window/Event.hpp"

//!!
//!! "Tag" this file for not just SFML dependency, but also for Windows!
//!!

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h> // GetKeyState(), VK_... for the Num/Caps/Scroll lock keys
#else
#  error Only Windows is supported yet!
#endif

#include "extern/iprof/iprof.hpp" //!! Don't leave this here forever!...

namespace UI {

// Call this once, somewhere in the (beginning of the) app's SFML event loop!
void update_keys(const sf::Event& sfml_event);
//!! Note: ideally, other UI backends would just need their own overloads on their specific event type.

// SFML -> SimApp virt. keycode translation
constexpr auto key_from_SFML(auto sfml_keycode);

//!! Simple, but FRAGILE map from SFML keys to ours -- SFML codes can change at any time!
//!! (Especially as they haven't released a stable v3 API yet!)
constinit enum VKEY _SFML_KBD_XLAT[] = {
	VKEY::A, VKEY::B, VKEY::C, VKEY::D, VKEY::E, VKEY::F, VKEY::G, VKEY::H,
		VKEY::I, VKEY::J, VKEY::K, VKEY::L, VKEY::M, VKEY::N, VKEY::O, VKEY::P,
	VKEY::Q, VKEY::R, VKEY::S, VKEY::T, VKEY::U, VKEY::V, VKEY::W, VKEY::X,
		VKEY::Y, VKEY::Z, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::ESCAPE, VKEY::NUL, VKEY::LCTRL, VKEY::LSHIFT, VKEY::LALT,
		VKEY::WINDOWS, VKEY::RCTRL, VKEY::RSHIFT, VKEY::RALT, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::TILDE, VKEY::NUL,
		VKEY::NUL, VKEY::SPACE, VKEY::NUL, VKEY::NUL, VKEY::TAB, VKEY::NUL, VKEY::NUL, VKEY::NUL,

	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::LEFT,
		VKEY::RIGHT, VKEY::UP, VKEY::DOWN, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::F1, VKEY::F2, VKEY::F3, VKEY::F4,
		VKEY::F5, VKEY::F6, VKEY::F7, VKEY::F8, VKEY::F9, VKEY::F10, VKEY::F11, VKEY::F12,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::PAUSE, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,

	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,

	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
	VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
		VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL, VKEY::NUL,
};

} // namespace UI


//--------------------------------------------------------------------------------------
// Implementation...
//--------------------------------------------------------------------------------------

inline constexpr auto UI::key_from_SFML(auto sfml_keycode)
{
	return _SFML_KBD_XLAT[sfml_keycode];
}

inline void UI::update_keys(const sf::Event& sfml_event)
//!!
//!! If GetKeyState is really a "real" kernel call (as opposed to just a DLL function),
//!! then it's probably horrendously expensive to call en masse, like no tomorrow! :-o
//!!
// (No need for a separate transl. unit for this, as it's supposed to be called from only one.)
{
//!!??IPROF_FUNC; -> #335

//!! Should be locked to prevent any interleaving state changes via a parallel
//!! call to reset_keys()!

	//!!?? How to do these with SFML?
	_kbd_state[VKEY::NUMPAD_ENTER]  = (unsigned)GetKeyState(VK_RETURN) & 0xff80;
	_kbd_state[VKEY::NUMPAD_PLUS]   = (unsigned)GetKeyState(VK_ADD) & 0xff80;
	_kbd_state[VKEY::NUMPAD_MINUS]  = (unsigned)GetKeyState(VK_SUBTRACT) & 0xff80;
	_kbd_state[VKEY::NUMPAD_0]      = (unsigned)GetKeyState(VK_NUMPAD0) & 0xff80;
	_kbd_state[VKEY::NUMPAD_DOT]    = (unsigned)GetKeyState(VK_DECIMAL) & 0xff80;

	switch (sfml_event.type)
	{
	case sf::Event::KeyReleased:
		if (sfml_event.key.code != -1) {
			_kbd_state[key_from_SFML((unsigned)sfml_event.key.code % (unsigned)VKEY::__SIZE__)] = false;
		} else {
//cerr << "UNKNOWN-TO-SFML KEY (-1)...\n"; // Incl. all the ...Locks :-/
			// Emulate KeyReleased for the ...Locks ignored by SFML:
			_kbd_state[VKEY::CAPS_LOCK]   = _kbd_state[VKEY::CAPS_LOCKED] != (bool)((unsigned)GetKeyState(VK_CAPITAL) & 1);
			_kbd_state[VKEY::NUM_LOCK]    = _kbd_state[VKEY::NUM_LOCKED] != (bool)((unsigned)GetKeyState(VK_NUMLOCK) & 1);
			_kbd_state[VKEY::SCROLL_LOCK] = _kbd_state[VKEY::SCROLL_LOCKED] != (bool)((unsigned)GetKeyState(VK_SCROLL) & 1);
		}
		break;

	case sf::Event::KeyPressed:
		if (sfml_event.key.code != -1) {
			_kbd_state[key_from_SFML((unsigned)sfml_event.key.code % (unsigned)VKEY::__SIZE__)] = true;
		} else {
//cerr << "UNKNOWN-TO-SFML KEY (-1)...\n"; // Incl. all the ...Locks :-/
			// Emulate KeyPressed for the ...Locks ignored by SFML:
			_kbd_state[VKEY::CAPS_LOCK]   = (unsigned)GetKeyState(VK_CAPITAL) & 0xff80;
			_kbd_state[VKEY::NUM_LOCK]    = (unsigned)GetKeyState(VK_NUMLOCK) & 0xff80;
			_kbd_state[VKEY::SCROLL_LOCK] = (unsigned)GetKeyState(VK_SCROLL) & 0xff80;
		}
	default:;
	}

	// Set some virtual key meta-states:
	_kbd_state[VKEY::SHIFT] = _kbd_state[VKEY::LSHIFT] || _kbd_state[VKEY::RSHIFT]; //!! SFML/Windows BUG: https://github.com/SFML/SFML/issues/1301
	_kbd_state[VKEY::CTRL]  = _kbd_state[VKEY::LCTRL]  || _kbd_state[VKEY::RCTRL];
	_kbd_state[VKEY::ALT]   = _kbd_state[VKEY::LALT]   || _kbd_state[VKEY::RALT];

//!!?? Is this still needed, given the switch above?
//!!?? Or, IOW: why is that one needed, if we have this below?
//!! (I vaguely recall the Win32 API being more cumbersome than just letting me
//!! do the straightforward thing, but can't remember what the pitfalls were!)
	// These are ignored by SFML, must get them manually:
	_kbd_state[VKEY::CAPS_LOCKED]   = (unsigned)GetKeyState(VK_CAPITAL) & 1;
	_kbd_state[VKEY::NUM_LOCKED]    = (unsigned)GetKeyState(VK_NUMLOCK) & 1;
	_kbd_state[VKEY::SCROLL_LOCKED] = (unsigned)GetKeyState(VK_SCROLL) & 1;
}

#endif //_UI_SFML_keycodes_hpp_
