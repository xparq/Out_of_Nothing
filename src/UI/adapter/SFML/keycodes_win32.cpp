#include "keycodes.hpp"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h> // GetKeyState(), VK_... for the Num/Caps/Scroll lock keys
#else
#  error Only Windows is supported yet!
#endif

#include "extern/iprof/iprof.hpp" //!! Don't leave this here forever!...

namespace UI {


//----------------------------------------------------------------------------
// REMEMBER: THIS FUNCTION IS CALLED IN EVERY FRAME. DON'T BLOAT IT!
//----------------------------------------------------------------------------
void update_keys_from_SFML(const sf::Event& sfml_event)
//!!
//!! I haven't found any hints about that but if GetKeyState still happens to
//!! involve a kernel call (as opposed to just being a DLL function), then it'd
//!! be horrendously expensive, and shouldn't be called en masse like no tomorrow! :-o
//!!
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
		if (sfml_event.key.code != sf::Keyboard::Key::Unknown) {
			_kbd_state[_keycode_from_SFML((unsigned)sfml_event.key.code % (unsigned)VKEY::__SIZE__)] = false;
		} else {
//cerr << "UNKNOWN-TO-SFML KEY (-1)...\n"; // Incl. all the ...Locks :-/
			// Emulate KeyReleased for the ...Locks ignored by SFML:
			_kbd_state[VKEY::CAPS_LOCK]   = _kbd_state[VKEY::CAPS_LOCKED] != (bool)((unsigned)GetKeyState(VK_CAPITAL) & 1);
			_kbd_state[VKEY::NUM_LOCK]    = _kbd_state[VKEY::NUM_LOCKED] != (bool)((unsigned)GetKeyState(VK_NUMLOCK) & 1);
			_kbd_state[VKEY::SCROLL_LOCK] = _kbd_state[VKEY::SCROLL_LOCKED] != (bool)((unsigned)GetKeyState(VK_SCROLL) & 1);
		}
		break;

	case sf::Event::KeyPressed:
		if (sfml_event.key.code != sf::Keyboard::Key::Unknown) {
			_kbd_state[_keycode_from_SFML((unsigned)sfml_event.key.code % (unsigned)VKEY::__SIZE__)] = true;
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

} // update_keys

} // namespace UI