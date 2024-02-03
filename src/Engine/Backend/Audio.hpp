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
	struct PlayReq // Playing options -- (kinda) used to find the optimal free channel
	{
		short priority = 0; // Higher-priority sounds can cut lower ones in case of congestion.
		bool loop = false;  // If true, priority will also be incremented by 1. (Set that to one less to compensate!)
		short effective_priority() const { return priority + short(loop); }

		unsigned short sample_rate = 0; // Use whatever the sound object (buffer) itself defined!
	};

	static constexpr PlayReq DefaultPlayMode = {.loop = false, .sample_rate = 0};

public:
	virtual bool   toggle_audio()  { return sz::toggle(&enabled); }
	virtual bool   toggle_music()  { return sz::toggle(&music_enabled); }
	virtual bool   toggle_sounds() { if (!sz::toggle(&fx_enabled)) { kill_sounds(); } return fx_enabled; }

	virtual void   volume(float)  {}
	virtual float  volume() const { return 0; }

	virtual bool   play_music(const char* /*filename*/) { return false; }

	constexpr static size_t INVALID_SOUND_BUFFER = ~0u;
	virtual size_t add_sound(const char* /*filename*/)  { return INVALID_SOUND_BUFFER; } // Returns buffer #
	virtual float  length(size_t /*buffer*/) const { return 0; } // 0 for invalid input

	constexpr static short INVALID_SOUND_CHANNEL = -1;
	virtual short  play_sound(size_t /*buffer*/, [[maybe_unused]] PlayReq options = DefaultPlayMode) { return INVALID_SOUND_CHANNEL; } // Returns channel #
	virtual bool   playing(short /*channel*/) const { return false; }
	virtual void   kill_sound(short /*channel*/) {}; // Tolerates channel == INVALID_SOUND_CHANNEL
	virtual void   kill_sounds() {}
	virtual void   toggle_sound(size_t /*buffer*/) {}

	// --- C++ cruft -----------------------------------------------------
	virtual ~Audio() = default;
}; // class Audio

} // namespace Szim

#endif // _KJLK3JLXWPOCMRUVTNBYIY784702_
