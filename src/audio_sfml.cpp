#include "audio_sfml.hpp"

#ifndef DISABLE_AUDIO // Defined in cfg.h, which may come via the audio header, so the #include should be outside this #ifdef!... ;)

#include <SFML/Audio/Sound.hpp>

#include <iostream>
	using std::cerr, std::endl;


size_t Audio_SFML::add_sound(const char* filename)
{
//!!	sounds.emplace_back(); //! Emplace would *STILL* call a copy ctor, if no args! :-o So fkn' disappointing! :(
//!!!!	sounds.resize(sounds.size() + 1); //! Oh, wow, *EVEN* this one will insist on copying!! :-o WTF?! :(
	sounds.emplace_back(0); //! If it has an argument, *then* it finally won't copy... Phew.

	if (auto& last_slot = sounds[sounds.size() - 1];
		!last_slot.loadFromFile(filename)) {
cerr << "- Error loading sound: " << filename << endl;
		return 0; //! item #0 is for errors like this
	}

	return sounds.size() - 1;
}

void Audio_SFML::play_sound(size_t ndx)
{
	assert(ndx < sounds.size());
	// Also check run-time, to be absolutely sure:
	if (ndx >= sounds.size()) {
		return;
	}
	if (audio_enabled && !sounds[ndx].muted) {
		_sound.setBuffer(sounds[ndx]);
		_sound.play();
	}
}

bool Audio_SFML::play_music(const char* filename)
{
	if (!_music.openFromFile(filename)) {
cerr << "- Error loading music: " << filename << endl;
		return false;
	}
	if (audio_enabled) {
		_music.setLoop(true);
		_music.play();
	}
	return true;
}

//!!NOTE: This one pauses, vs. toggle_sound() mutes!
void Audio_SFML::toggle_music()
{
	if (_music.getStatus() == sf::Music::Playing) {
		_music.pause();
	} else {
		_music.play();
	}
}

//!!NOTE: This one mutes, vs. toggle_music() pauses!
void Audio_SFML::toggle_sound(size_t ndx)
{
	if (ndx >= sounds.size()) {
		return;
	}
	sounds[ndx].muted = !sounds[ndx].muted;
}

#endif // DISABLE_AUDIO
