#ifndef _04857BVNC2V4687N89MX4567N9845667B89CX6MV4778M78_
#define _04857BVNC2V4687N89MX4567N9845667B89CX6MV4778M78_

namespace UI {

//------------------------------------------------------------------------
//!!Integrate it as sg. like "UI/input/controller/button_state", to uniformly
//!!support more than just a keyboard (mouse, touch, joys, game controllers etc.).
//!!Virtual tap buttons should also be possible to route thru the successor of this ifc.
//!!And then add support for non-latin keyboards, too!...
//!!
//!!Also: provide conversion key code conversion tables for various supported backends (like SFML, SDL, Win32 etc.)
//!!(The same mechanism could also be extended even further to allow layout remapping independently of what's
//!!supported by those.)
//!!
//!! -> But note: such conversions should be adapter add-ons,
//!!    to avoid introducing dependencies into the base package!

enum VKEY : unsigned {
	//! -> eg. https://developer.mozilla.org/en-US/docs/Web/API/UI_Events/Keyboard_event_key_values
	//!    or: http://www.kbdedit.com/manual/low_level_vk_list.html

	//! Adding typically non-existent "keys" too, both to follow ASCII,
	//! and as placeholders for layouts that actually do have them.
	//! (And this is our own virtual keyboard anyway, we can do with it whatever we want. ;-p )

	NUL = 0,
	PAUSE = 1,
	
	//!!CONTEXT[_MENU],
	//!!Keypad...
	//!!Media keys...
	//!!Accented keys, obscure labels on "extended" keyboards

	HOME = 2, END,
	UP = 4, DOWN, LEFT, RIGHT,

	BACKSPACE = 8,
	TAB = 9,
	INSERT = 10,
	PAGE_UP = 11, PAGE_DOWN,
	ENTER = 13,

	LSHIFT = 15, RSHIFT, // SHIFT is a meta state, see below
	LCTRL = 18, RCTRL, // CTRL is a meta state, see below
	LALT = 21, RALT, ALTGR = RALT, // ALT is a meta state, see below
	WINDOWS,

	CAPS_LOCK, NUM_LOCK, SCROLL_LOCK, //!!Emulated in OON_sfml now -- move to a non-OON, but SFML-specific event preprocessor!
										//! NOTE: CAPS, NUM & SCROLL give key.code -1 in SFML 2.6.x :-/
	ESCAPE = 27,

	SPACE = 32,

	EXCLAMATION,
	QUOTE,
	NUMBER,
	DOLLAR,
	PERCENT,
	AMPERSAND,
	APOSTROPHE,
	PAREN_LEFT,
	PAREN_RIGHT,
	ASTERISK,
	PLUS,

	COMMA,
	HYPHEN,
	DOT,
	SLASH,					 
	DIGIT0 = 48, DIGIT1, DIGIT2, DIGIT3, DIGIT4, DIGIT5, DIGIT6, DIGIT7, DIGIT8, DIGIT9,
	COLON, SEMICOLON, LESS_THAN, EQUALS, GREATER_THAN, QUESTION_MARK, AT,
	A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	BRACKET_LEFT, BACKSLASH, BRACKET_RIGHT, CARET, UNDERSCORE, BACKQUOTE,
	a = 97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
	BRACE_LEFT, PIPE, BRACE_RIGHT, TILDE,
	DELETE = 127, 
	F1 = 128, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, // F16 = 143

	// Keypad:
	NUMPAD_ENTER = 144, NUMPAD_PLUS, NUMPAD_MINUS, NUMPAD_MULTIPLY, NUMPAD_DIVIDE, NUMPAD_DOT,
	NUMPAD_0, NUMPAD_1, NUMPAD_2, NUMPAD_3, NUMPAD_4, NUMPAD_5, NUMPAD_6, NUMPAD_7, NUMPAD_8, NUMPAD_9, // NUMPAD_9 = 159

	// Meta states:
	SHIFT = 160, CTRL, ALT,
	CAPS_LOCKED, NUM_LOCKED, SCROLL_LOCKED,

	__LAST__ = 254, __SIZE__ //! Should be good for a while...
};


extern volatile bool _kbd_state[];

inline auto keystate(VKEY k) { return _kbd_state[k]; }

//!!inline void update_keystates() { return update_keys(...); }
void reset_keys();

} // namespace UI

#endif // _04857BVNC2V4687N89MX4567N9845667B89CX6MV4778M78_
