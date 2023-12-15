#include "_Audio.hpp"

//============================================================================
// SFML-BACKED AUDIO SUBSYSTEM - IMPLEMENTATION
//============================================================================

#ifndef DISABLE_AUDIO

#include <iostream>
	using std::cerr, std::endl;


namespace Szim {

//----------------------------------------------------------------------------
SFML_Audio::buffer_wrapper::buffer_wrapper(const char* filename)
{
	if (!loadFromFile(filename)) {
std::cerr << "- Error loading sound from " << filename << std::endl;
	}
	else empty = false;
}


//----------------------------------------------------------------------------
SFML_Audio::SFML_Audio()
{
	// Add an empty element so if add() returns 0 for errors it could still
	// be used as an index, pointing to a safe & silent spot.
//	_sound_buffer_autoptrs.resize(1); //! Remember the implicit copy ctor call here (no matter what)! :-o (-> also: add_sound!)
}

//----------------------------------------------------------------------------
Audio& SFML_Audio::get()
{
	static SFML_Audio audio{};
	return audio;
}


//----------------------------------------------------------------------------
bool SFML_Audio::toggle_audio()
{
	if (Audio::toggle_audio()) {
		if (_music.getStatus() != sf::Music::Playing) // (Re)start if paused/stopped/never started
			if (music_enabled) _music.play();
	} else {
		_music.pause(); //! not stop()
		kill_sounds();
	}
	return enabled;
}


//----------------------------------------------------------------------------
void SFML_Audio::volume(float vol)
{
	_master_volume = vol;
	_music.setVolume(_master_volume);
}


//----------------------------------------------------------------------------
float SFML_Audio::volume() const
{
	return _master_volume;
	//!! Not this, as this might be adjusted/normalized: return _music.getVolume();
}


//----------------------------------------------------------------------------
bool SFML_Audio::play_music(const char* filename)
{
	if (!_music.openFromFile(filename)) {
cerr << "- Error loading music: " << filename << endl;
		return false;
	}
	_music.setLoop(true);

	if (enabled && music_enabled) {
		_music.play();
	}
	return true;
}

//----------------------------------------------------------------------------
//!!NOTE: This one pauses, vs. toggle_sound() mutes!
bool SFML_Audio::toggle_music()
{
	if (Audio::toggle_music()) {
		if (enabled && _music.getStatus() != sf::Music::Playing) // (Re)start if paused/stopped/never started?
			_music.play();
	} else {
		_music.pause();
	}
	return music_enabled;
}



//----------------------------------------------------------------------------
//!! Mature this up to the generic API! (Needs sensible def. of the total channels!)
short SFML_Audio::_get_sound_channel_ndx(const PlayReq& opt)
//	short  play_sound(size_t, PlayReq options = DefaultPlayMode) override;
//
// - The round-robin (not really "LRU"...) index advances by one upon every call to this.
// - The newly allocated slot may advance by more than one, in case the LRU slot had priority!
//
{
#define _next(chn) ( ((chn) + 1) % MAX_FX_CHANNELS )

	short slot = _LRU_channel;
	_LRU_channel = _next(_LRU_channel);

	// It's crucial to run the loop below at least once (unless there's only one channel),
	// to advance the LRU index!

	if (!opt.loop) { // Only looped sounds need prioritizing (for now) -> #427
		for (short tries = MAX_FX_CHANNELS - 1; _fx_channels[slot].getLoop() && tries > 0; --tries) {
			// Oh, and BTW, if we are at it... :) Check if the last 'slot' candidate has stopped
			// playing, because that'd be a better LRU index than "guess by increment"! :)
			//!!Except: no... It's perfect for the next sound, but then, in the subsequent
			//!!round (right after that), it would just go ahead and kill the one we're
			//!!starting right now...
			//!!if (_fx_channels[slot].getStatus() == sf::Sound::Stopped)
			//!!	_LRU_channel = slot;

			// Try another slot:
			slot = _next(slot);
		}
	}
	assert(_LRU_channel != slot || _LRU_channel == slot && MAX_FX_CHANNELS == 1);

	return slot;

#undef _next_channel
}


//----------------------------------------------------------------------------
SoundPlayer& SFML_Audio::_get_channel(short ndx)
{
	assert(ndx >= 0 && ndx < MAX_FX_CHANNELS);
	//!! Albeit an internal call, also check run-time, while things are unstable:
	if (ndx < 0 || ndx >= MAX_FX_CHANNELS) {
cerr << "DBG> _get_channel: ERROR: invalid channel index #"<<ndx<<"! :-o \n";
		return _fx_channels[0];
	}

	return _fx_channels[ndx];
}

//----------------------------------------------------------------------------
size_t SFML_Audio::add_sound(const char* filename)
{
	_sound_buffer_autoptrs.emplace_back(std::make_shared<buffer_wrapper>(filename));

	size_t buffer_ndx = _sound_buffer_autoptrs.size() - 1;

	if (_sound_buffer_autoptrs.back()->empty) {
		_sound_buffer_autoptrs.pop_back();
cerr << "- Error loading sound #"<< buffer_ndx <<" from "<< filename <<'\n';
cerr << "DBG> NOTE: Removing empty sound after load failure.\n";
		return INVALID_SOUND_BUFFER;
	}

//	play_sound(buffer_ndx);
cerr << "DBG> SFML_Audio::add_sound Loaded buffer #"<<buffer_ndx<<" from file: "<<filename<<"\n"
     << "DBG> - sample rate: "<<_sound_buffer_autoptrs.back()->getSampleRate()
     <<         ", channels: "<<_sound_buffer_autoptrs.back()->getChannelCount()<<'\n';
	return buffer_ndx;
}

//----------------------------------------------------------------------------
short SFML_Audio::play_sound(size_t buffer_ndx, PlayReq options)
{
	if (buffer_ndx >= _sound_buffer_autoptrs.size()) {
cerr << "DBG> play_sound: WARNING: invalid sound index #"<<buffer_ndx<<" ignored.\n";
		return INVALID_SOUND_CHANNEL;
	}
	if (!enabled || !fx_enabled ||
		_sound_buffer_autoptrs[buffer_ndx]->empty ||
		_sound_buffer_autoptrs[buffer_ndx]->muted)
	{
		return INVALID_SOUND_CHANNEL;
	}

	auto channel_ndx = _get_sound_channel_ndx(options);
	auto& channel = _get_channel(channel_ndx);
	channel.stop(); // May be playing some other channel! (May not be a problem for SFML tho.)
	channel.setBuffer(*(_sound_buffer_autoptrs[buffer_ndx]));
//cerr << "DBG> play_sound: setting channel #"<<channel_ndx<<" to buffer #"<<buffer_ndx<<" for playing\n";
	channel.setVolume(_master_volume);
	channel.setLoop(options.loop);
//!!No (need for?) explicit resampling for SFML:
//!!	channel.setSesampleRate(options.loop);
	channel.play();

	return channel_ndx;
}

//----------------------------------------------------------------------------
// NOTE: This one mutes, whereas toggle_music() pauses!
void SFML_Audio::toggle_sound(size_t buffer_ndx)
{
	if (buffer_ndx >= _sound_buffer_autoptrs.size()) {
cerr << "DBG> toggle_sound: WARNING: invalid sound index #"<<buffer_ndx<<" ignored.\n";
		return;
	}
	_sound_buffer_autoptrs[buffer_ndx]->muted = !_sound_buffer_autoptrs[buffer_ndx]->muted;
}

//----------------------------------------------------------------------------
void SFML_Audio::kill_sound(short channel_ndx)
{
	if (channel_ndx < 0 || channel_ndx >= MAX_FX_CHANNELS) {
cerr << "DBG> kill_sound: WARNING: invalid channel index #"<<channel_ndx<<" ignored.\n";
		return;
	}

	auto& channel = _get_channel(channel_ndx);
	channel.stop();
}

//----------------------------------------------------------------------------
void SFML_Audio::kill_sounds()
{
	for (auto& ch : _fx_channels) {
		ch.stop();
	}
}

} // namespace Szim

#endif // DISABLE_AUDIO