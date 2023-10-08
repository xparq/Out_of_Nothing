#ifndef _AUDIO_SFML_
#define _AUDIO_SFML_

#include "sz/toggle.hh"

#include <cstddef> // size_t

class Audio_Stub
{
public:
	sz::Toggle enabled = true; // all-audio override
	sz::Toggle music_enabled = true;
	sz::Toggle fx_enabled = true;
public:
	virtual bool   play_music(const char* filename) { filename; return false; }
	virtual size_t add_sound(const char* filename)  { filename; return 0; }
	virtual void   play_sound(size_t ndx)  { ndx; }
	virtual bool   toggle_audio()  { return sz::toggle(&enabled); }
	virtual bool   toggle_music()  { return sz::toggle(&music_enabled); }
	virtual bool   toggle_sounds(); // fx
	virtual void   toggle_sound(size_t) {} // fx
	virtual void   kill_sounds() {} // fx
};

#ifndef DISABLE_AUDIO // If disabled, only the stub class will be available.

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>

#include <vector>

class Audio_SFML : public Audio_Stub
{
	struct SndBuf_NoCopy_Wrapper_thanksfornothing_std_vector : public sf::SoundBuffer {
		bool muted = false;

		SndBuf_NoCopy_Wrapper_thanksfornothing_std_vector(int) {}
		SndBuf_NoCopy_Wrapper_thanksfornothing_std_vector() {}
		SndBuf_NoCopy_Wrapper_thanksfornothing_std_vector(const SndBuf_NoCopy_Wrapper_thanksfornothing_std_vector&)
			{ /* cerr << "SFML SndBuf wrapper BEING COPIED!\n"; */ }
	};
	std::vector<SndBuf_NoCopy_Wrapper_thanksfornothing_std_vector> sounds;

public:
	bool   play_music(const char* filename) override;
	size_t add_sound(const char* filename) override;
	void   play_sound(size_t ndx) override;
	bool   toggle_audio() override;
	bool   toggle_music() override;
	void   toggle_sound(size_t ndx) override;
	void   kill_sounds() override;

	Audio_SFML(): _dummy_soundbuffer_for_SFML(), _sound(_dummy_soundbuffer_for_SFML)
	{
		// Add an empty element so if add() returns 0 for errors it could still
		// be used as an index, pointing to a safe & silent spot.
		sounds.resize(1); //! Remember the implicit copy ctor call here (no matter what)! :-o (-> also: add_sound!)
	}
private:
	sf::SoundBuffer _dummy_soundbuffer_for_SFML; //! Sigh, SFML/9cbcd56... :-(
	sf::Sound _sound; //!! only this one single player object yet!
	sf::Music _music; //!! only this one single player object yet!
};

#endif // DISABLE_AUDIO

#endif // _AUDIO_SFML_
