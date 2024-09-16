#ifndef _XMHJ9UV5YW4985NVF46Y357KJ9_
#define _XMHJ9UV5YW4985NVF46Y357KJ9_

//#include "sz/toggle.hh"
#include <cstddef> // size_t
#include "Engine/Backend/Audio.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>

#include <memory> // sf::SoundBuffers don't survive being directly added to reallocatable
                  // containers (like std::vector), so must be wrapped into e.g. shared_ptr!
                  // (SFML aborts (at least) in DEBUG mode if a sound is being playing while
                  // its source vector<sf::SoundBuffer> changes & reallocates!)
#include <vector>

#include "sz/diag/DBG.hh"

namespace Szim {

//!! Sigh, amending the botched sf::Sound API that has no default ctor any more, as of SFML/9cbcd56... :-/
struct _cpp_cringe_base_from_other_base : sf::SoundBuffer { using sf::SoundBuffer::SoundBuffer; };
struct SoundPlayer : _cpp_cringe_base_from_other_base, sf::Sound {

	using sf::Sound::Sound;
	SoundPlayer() : sf::Sound(_SFML_cringe_get_dummy_buffer()) {}
	private: sf::SoundBuffer& _SFML_cringe_get_dummy_buffer() { return (sf::SoundBuffer&) *this; }
	SoundPlayer(const SoundPlayer&) = delete;

//!! OK then, if things are this bad, so we're kinda forced to a) have this class,
//!! b) use shared_ptr, then let's make this class a useful abstraction:
//!!	...
//!!	play(buffer_wrapper& sndbuf) { setBuffer((sf::SoundBuffer&)sndbuf); play(); }
//!!	shared_ptr<buffer_wrapper> buffer_playing;
//!!
//!! Oh, but then it should also derive from an API-level abstraction, via TRIPLE inheritance! :-o :)
public:
	bool playing() const { return getStatus() == sf::Sound::Status::Playing; } //!!?? or != sf::Sound::Stopped?!

	short priority = 0;
};


class SFML_Audio : public Audio
{
	struct buffer_wrapper : public sf::SoundBuffer
	{
		bool empty = true;
		bool muted = false;
		float length = 0; // s
		unsigned short sample_rate = 0;

		buffer_wrapper(const char* filename);
		buffer_wrapper(const buffer_wrapper&) = delete;
	};

	std::vector< std::shared_ptr<buffer_wrapper> > _sound_buffer_autoptrs;

public:
	bool   toggle_audio() override;
	bool   toggle_music() override;

	void   volume(float) override;
	float  volume() const override;

	bool   play_music(const char* filename) override;

	size_t add_sound(const char* filename) override;
	float  length(size_t buffer_ndx) const override; //!! Add an overload for music, too!
	short  play_sound(size_t buffer_ndx, PlayReq options = DefaultPlayMode) override;

	void   kill_sound(short channel) override;
	void   kill_sounds() override;
	void   toggle_sound(size_t buffer) override;
	bool   playing(short channel) const override;

protected:
	short  _get_sound_channel_ndx(const PlayReq& options) const; // const&: always called on lvalues internally
	bool   _channel_ndx_valid(short ndx) const;
	SoundPlayer& _get_channel(short ndx); //!! Mature it up to the generic API!
	const SoundPlayer& _get_channel(short ndx) const { return ((SFML_Audio*)this)->_get_channel(ndx); }

private:
	float _master_volume = 75;

	sf::Music _music; //!! only this one single player object yet!

	//!! Not needed! sz::lockers<SoundPlayer, MAX_FX_CHANNELS> _fx_channels;
	static constexpr unsigned MAX_FX_CHANNELS = 10;
	SoundPlayer _fx_channels[MAX_FX_CHANNELS];
	short mutable _LRU_channel = 0; // Round-robin channel index

public:
	SFML_Audio();
}; // class SFML_Audio

} // namespace Szim
#endif // _XMHJ9UV5YW4985NVF46Y357KJ9_
