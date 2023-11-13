#include "Audio.hpp"

using namespace Szim;

bool   Audio::play_music(const char* filename) { filename; return false; }
size_t Audio::add_sound(const char* filename)  { filename; return 0; }
void   Audio::play_sound(size_t ndx)  { ndx; }
bool   Audio::toggle_audio()  { return sz::toggle(&enabled); }
bool   Audio::toggle_music()  { return sz::toggle(&music_enabled); }
bool   Audio::toggle_sounds() { if (!sz::toggle(&fx_enabled)) { kill_sounds(); } return fx_enabled; }
void   Audio::toggle_sound(size_t) {} // fx
void   Audio::kill_sounds() {} // fx
