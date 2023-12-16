#include "UI/Input.hpp"

//!! Ugh... Relay instead, like the rest of the backend adapters!
//!!??#include "UI/adapter/SFML/keycodes.hpp" // SFML -> SimApp keycode translation
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace UI {

//!! Should be more sophisticated and support reset_on_lost_focus/gained_focus,
//!! so that the local lock key states won't disturb other apps when switched
//!! away from here; and similarly, ours will be saved/restored, so foreign statas
//!! won't mess up the local ones here!
//!! (Assumes of course that the lock key states can be reliably set/reset.)
void reset_keys()
{
//!! Should be locked to prevent any interleaving state changes via a parallel
//!! call to update_keys()!
	for (auto i = 0; i <= VKEY::__LAST__; _kbd_state[i++] = false)
		;

	// The real lock keys can't just be zeroed, so...:
	//!!??update_keys()
	_kbd_state[VKEY::CAPS_LOCKED]   = (unsigned)GetKeyState(VK_CAPITAL) & 1;
	_kbd_state[VKEY::NUM_LOCKED]    = (unsigned)GetKeyState(VK_NUMLOCK) & 1;
	_kbd_state[VKEY::SCROLL_LOCKED] = (unsigned)GetKeyState(VK_SCROLL) & 1;
}

} // namespace UI
