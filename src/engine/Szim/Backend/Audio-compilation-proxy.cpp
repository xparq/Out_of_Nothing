/*!!
#include "_adapter_switcher.hpp"

#ifdef DISABLE_AUDIO
//!!#  include "Audio_stub.cpp.inc" // <- No need, everything's in the header currently!
#else
#  include SWITCHED(AUDIO_BACKEND, _Audio.cpp.inc)
#endif
!!*/