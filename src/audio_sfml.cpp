#include "audio_sfml.hpp"

#ifdef AUDIO_ENABLE // If disabled, only the stub class will be available!

#include <SFML/Audio/Sound.hpp>

#include <iostream>
using namespace std;


size_t Audio_SFML::add_sound(const char* filename)
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

void Audio_SFML::play_sound(size_t ndx)
{
	static sf::Sound sound; //!! only this one player yet!

	if (ndx >= sounds.size()) {
		return;
	}
	sound.setBuffer(sounds[ndx]);
	sound.play();
}

bool Audio_SFML::play_music(const char* filename)
{
	if (!_music.openFromFile(filename)) {
cerr << "- Error loading music: " << filename << endl;
		return false;
	}
	_music.setLoop(true);
	_music.play();
	return true;
}

void Audio_SFML::toggle_music()
{
	if (_music.getStatus() == sf::Music::Playing) {
		_music.pause();
	} else {
		_music.play();
	}
}

#endif // AUDIO_ENABLED
