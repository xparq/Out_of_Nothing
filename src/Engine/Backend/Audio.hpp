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
	virtual bool   toggle_audio()  { return sz::toggle(&enabled); }
	virtual bool   toggle_music()  { return sz::toggle(&music_enabled); }
	virtual bool   toggle_sounds() { if (!sz::toggle(&fx_enabled)) { kill_sounds(); } return fx_enabled; }

	virtual void   volume(float)  {}
	virtual float  volume() const { return 0; }

	virtual bool   play_music(const char* /*filename*/) { return false; }

	constexpr const static size_t INVALID_SOUND_BUFFER = ~0u;
	virtual size_t add_sound(const char* /*filename*/)  { return INVALID_SOUND_BUFFER; } // Returns buffer #
	constexpr const static short INVALID_SOUND_CHANNEL = -1;
	virtual short  play_sound(size_t, [[maybe_unused]] bool loop = false) { return INVALID_SOUND_CHANNEL; } // Returns channel #
	virtual void   kill_sound(short /*channel*/) {}; // Tolerates channel == INVALID_SOUND_CHANNEL
	virtual void   kill_sounds() {}
	virtual void   toggle_sound(size_t /*buffer*/) {}

	virtual ~Audio() = default;

	// Must be implemented by derived classes:
	//static Audio* get() { static_assert(false, "::get() should be supplied by implementation classes!"); }
}; // class Audio

} // namespace Szim

#endif // _KJLK3JLXWPOCMRUVTNBYIY784702_
