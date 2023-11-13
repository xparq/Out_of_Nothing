#include "Audio.hpp"

using namespace Szim;

#ifdef DISABLE_AUDIO
//!! No need, currently everything is in the header:
//!!#  include "Audio_stub.cpp.inc"
#else
#  include "Backend/SFML/Audio_SFML.cpp.inc"
#endif
