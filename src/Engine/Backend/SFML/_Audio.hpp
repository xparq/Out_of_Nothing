#ifndef _XMHJ9UV5YW4985NVF46Y357KJ9_
#define _XMHJ9UV5YW4985NVF46Y357KJ9_

#include "sz/toggle.hh"
#include <cstddef> // size_t
#include "Engine/Audio.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>

#include <vector>

namespace Szim {

class SFML_Audio : public Audio
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
	void   volume(float) override;
	float  volume() const override;


	SFML_Audio(): _dummy_soundbuffer_for_SFML{}, _sound{_dummy_soundbuffer_for_SFML}
	{
		// Add an empty element so if add() returns 0 for errors it could still
		// be used as an index, pointing to a safe & silent spot.
		sounds.resize(1); //! Remember the implicit copy ctor call here (no matter what)! :-o (-> also: add_sound!)
	}
private:
	float _master_volume = 75;
	sf::SoundBuffer _dummy_soundbuffer_for_SFML; //! Sigh, SFML/9cbcd56... :-(
	sf::Sound _sound; //!! only this one single player object yet!
	sf::Music _music; //!! only this one single player object yet!

public:
	static Audio& get();
}; // class SFML_Audio

} // namespace Szim
#endif // _XMHJ9UV5YW4985NVF46Y357KJ9_
