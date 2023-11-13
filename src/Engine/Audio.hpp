#ifndef _KJLK3JLXWPOCMRUVTNBYIY784702_
#define _KJLK3JLXWPOCMRUVTNBYIY784702_

#include "sz/toggle.hh"
#include <cstddef> // size_t

namespace Szim {

class Audio
{
public://!! <- protected
	sz::Toggle enabled = true; // all-audio override
	sz::Toggle music_enabled = true;
	sz::Toggle fx_enabled = true;
public:
	virtual bool   play_music(const char* filename) { filename; return false; }
	virtual size_t add_sound(const char* filename)  { filename; return 0; }
	virtual void   play_sound(size_t) {}
	virtual void   toggle_sound(size_t) {}
	virtual void   kill_sounds() {}

	virtual bool   toggle_audio()  { return sz::toggle(&enabled); }
	virtual bool   toggle_music()  { return sz::toggle(&music_enabled); }
	virtual bool   toggle_sounds() { if (!sz::toggle(&fx_enabled)) { kill_sounds(); } return fx_enabled; }

	virtual ~Audio() = default;

	// Must be implemented by derived classes:
	//static Audio* get() { static_assert(false, "::get() should be supplied by implementation classes!"); }
}; // class Audio

} // namespace Szim

#endif // _KJLK3JLXWPOCMRUVTNBYIY784702_
