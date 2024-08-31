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
		return; // 'empty' remains true
	}

	empty = false;
	length = getDuration().asSeconds();
	sample_rate = (decltype(sample_rate)) getSampleRate();
}


//----------------------------------------------------------------------------
SFML_Audio::SFML_Audio()
{
	// Add an empty element so if add() returns 0 for errors it could still
	// be used as an index, pointing to a safe & silent spot.
//	_sound_buffer_autoptrs.resize(1); //! Remember the implicit copy ctor call here (no matter what)! :-o (-> also: add_sound!)
}


//----------------------------------------------------------------------------
bool SFML_Audio::toggle_audio()
{
	if (Audio::toggle_audio()) {
		if (_music.getStatus() != sf::Music::Status::Playing) // (Re)start if paused/stopped/never started
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
		if (enabled && _music.getStatus() != sf::Music::Status::Playing) // (Re)start if paused/stopped/never started?
			_music.play();
	} else {
		_music.pause();
	}
	return music_enabled;
}



//----------------------------------------------------------------------------
//!! Mature this up to the generic API! (Needs sensible def. of the total channels!)
short SFML_Audio::_get_sound_channel_ndx(const PlayReq& opt) const
//	short  play_sound(size_t, PlayReq options = DefaultPlayMode) override;
//
// - The round-robin (not really "LRU"...) index advances by one upon every call to this.
// - The newly allocated slot may advance by more than one, in case the LRU slot had priority!
//
{
#define _next(chn) ( ((chn) + 1) % MAX_FX_CHANNELS )

	short slot = _LRU_channel;
	_LRU_channel = _next(_LRU_channel);

	auto req_priority = opt.effective_priority();
	for (short retries = MAX_FX_CHANNELS - 1;
	     _fx_channels[slot].playing() && _fx_channels[slot].priority > req_priority && retries > 0;
	     --retries) {
		// Oh, and BTW, if we are at it... :) Check if the last 'slot' candidate has stopped
		// playing, because that'd be a better LRU index than "guess by increment"! :)
		//!!Except: no... It's perfect for the next sound, but then, in the subsequent
		//!!round (right after that), it would just go ahead and kill the one we're
		//!!starting right now...
		//!!if (_fx_channels[slot].getStatus() == sf::Sound::Stopped)
		//!!	_LRU_channel = slot;

		// Try another slot:
		slot = _next(slot);
		_LRU_channel = _next(_LRU_channel);
	}

	assert(_LRU_channel != slot || _LRU_channel == slot && MAX_FX_CHANNELS == 1);

	assert (_channel_ndx_valid(slot));
	return slot;

#undef _next_channel
}

//----------------------------------------------------------------------------
inline bool SFML_Audio::_channel_ndx_valid(short ndx) const
{
	return ndx >= 0 && ndx < MAX_FX_CHANNELS;
}


//----------------------------------------------------------------------------
SoundPlayer& SFML_Audio::_get_channel(short ndx)
{
	assert(_channel_ndx_valid(ndx));
	//!! Albeit an internal call, also check run-time, while things are unstable:
	if (!_channel_ndx_valid(ndx)) {
cerr << "DBG> _get_channel: ERROR: invalid channel index "<<ndx<<"! :-o \n";
		return _fx_channels[0];
	}

	return _fx_channels[ndx];
}

//----------------------------------------------------------------------------
size_t SFML_Audio::add_sound(const char* filename)
{
	// Initialize new SFML SoundBuffer (or set buffer.empty = true):
	_sound_buffer_autoptrs.emplace_back(std::make_shared<buffer_wrapper>(filename));

	size_t buffer_ndx = _sound_buffer_autoptrs.size() - 1;

	if (_sound_buffer_autoptrs.back()->empty) { // Failed init?
		_sound_buffer_autoptrs.pop_back();
cerr << "- Error loading sound "<< buffer_ndx <<" from "<< filename <<'\n';
cerr << "DBG> NOTE: Removing empty sound after load failure.\n";
		return INVALID_SOUND_BUFFER;
	}

//	play_sound(buffer_ndx);
cerr << "DBG> "<<__FUNCTION__<<": Loaded buffer "<<buffer_ndx<<" from file: "<<filename<<"\n";
if(0)cerr << "DBG> - length: "<<_sound_buffer_autoptrs.back()->length<<"s"
     <<         ", sample rate: "<<_sound_buffer_autoptrs.back()->sample_rate
     <<         ", channels: "<<_sound_buffer_autoptrs.back()->getChannelCount()<<'\n';
	return buffer_ndx;
}


//----------------------------------------------------------------------------
float SFML_Audio::length(size_t buffer_ndx) const //override
{
	if (buffer_ndx >= _sound_buffer_autoptrs.size()) {
cerr << "DBG> "<<__FUNCTION__<<": WARNING: invalid sound index "<<buffer_ndx<<" ignored.\n";
		return 0;
	}

	return 	_sound_buffer_autoptrs[buffer_ndx]->empty ? 0
	      : _sound_buffer_autoptrs[buffer_ndx]->length;
}


//----------------------------------------------------------------------------
short SFML_Audio::play_sound(size_t buffer_ndx, PlayReq options)
{
	if (buffer_ndx >= _sound_buffer_autoptrs.size()) {
cerr << "DBG> "<<__FUNCTION__<<": WARNING: invalid sound index "<<buffer_ndx<<" ignored.\n";
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
	channel.priority = options.effective_priority();
	channel.setVolume(_master_volume);
	channel.setLoop(options.loop);
//!!No (need for?) explicit resampling for SFML:
//!!	channel.setSesampleRate(options.loop);
	channel.play();

	return channel_ndx;
}

//----------------------------------------------------------------------------
bool SFML_Audio::playing(short channel_ndx) const //override
{
	if (!_channel_ndx_valid(channel_ndx)) {
cerr << "DBG> "<<__FUNCTION__<<": WARNING: invalid channel index "<<channel_ndx<<"\n";
		return false; // All the API functions tolerate invalid inputs.
	}

	return _get_channel(channel_ndx).playing();
}

//----------------------------------------------------------------------------
void SFML_Audio::toggle_sound(size_t buffer_ndx)
//
// NOTE: This one mutes, whereas toggle_music() pauses!
//
//!! DELETE?
//!! - Is it used/needed at all?
//!! - Also, it can't really stop currently playing (i.e. long...) sounds!
{
	if (buffer_ndx >= _sound_buffer_autoptrs.size()) {
cerr << "DBG> toggle_sound: WARNING: invalid sound index "<<buffer_ndx<<" ignored.\n";
		return;
	}
	if ( false == (_sound_buffer_autoptrs[buffer_ndx]->muted = !_sound_buffer_autoptrs[buffer_ndx]->muted)) {
		// Stop a looped or other possibly long sound:
		//!!
		//!! Get player channels currently playing the buffer and stop them...
		//!!
	}
}

//----------------------------------------------------------------------------
void SFML_Audio::kill_sound(short channel_ndx)
{
	if (channel_ndx < 0 || channel_ndx >= MAX_FX_CHANNELS) {
cerr << "DBG> kill_sound: WARNING: invalid channel index "<<channel_ndx<<" ignored.\n";
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