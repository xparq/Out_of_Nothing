#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <vector>
using namespace std;

class Audio_SFML
{
	struct SndBuf_NoCopy_Wrapper_thanks_std_vector : public sf::SoundBuffer {
		SndBuf_NoCopy_Wrapper_thanks_std_vector(int) {}
		SndBuf_NoCopy_Wrapper_thanks_std_vector() {}
		SndBuf_NoCopy_Wrapper_thanks_std_vector(const SndBuf_NoCopy_Wrapper_thanks_std_vector&) { /* cerr << "SFML SndBuf wrapper WAS COPIED!\n"; */ }
	};
	vector<SndBuf_NoCopy_Wrapper_thanks_std_vector> sounds;

public:
	size_t add_sound(const char* filename)
	{
//!!	sounds.emplace_back(); //! Emplace would *STILL* call a copy ctor, if no args! :-o So fkn' stupid! :(
//!!!!	sounds.resize(sounds.size() + 1); //! Oh, wow, *EVEN* this will want to copy! :-o WTF?! :(
		sounds.emplace_back(0);

		if (auto& last_slot = sounds[sounds.size() - 1];
			!last_slot.loadFromFile(filename)) {
cerr << "- Error loading sound: " << filename << endl;
			return 0; //! item #0 is for errors like this
		}

		return sounds.size() - 1;
	}

	void play_sound(size_t ndx)
	{
		static sf::Sound sound; //!! only this one player yet!

		if (ndx >= sounds.size()) {
			return;
		}
		sound.setBuffer(sounds[ndx]);
		sound.play();
	}

	bool play_music(const char* filename)
	{
		if (!_music.openFromFile(filename)) {
cerr << "- Error loading music: " << filename << endl;
			return false;
		}
		_music.setLoop(true);
		_music.play();
		return true;
	}

	void toggle_music()
	{
		if (_music.getStatus() == sf::Music::Playing) {
			_music.pause();
		} else {
			_music.play();
		}
	}

	Audio_SFML()
	{
		// Add an empty element so if add() returns 0 for errors it could still
		// be used as an index, pointing to a safe & silent spot.
		sounds.resize(1); //! Remember the implicit copy ctor call here (no matter what)! :-o (-> also: add_sound!)
	}
private:
//!! If a default-constructed item works as expected, then no need for this:
//!!	sf::SoundBuffer default_dummy_no_sound;

	sf::Music _music; //!! only this one player yet!
};