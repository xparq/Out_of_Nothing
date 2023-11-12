#ifndef _VIY6OUAOIN4378G135Y_
#define _VIY6OUAOIN4378G135Y_

#include "Audio.hpp"

namespace Szim {

class Backend // Abstract Backend Base
{
public:
	Audio& audio; // Init. by the defived real backend!

public:
	virtual void SomeAPI() = 0;

protected:
	        Backend(Audio& real_audio): audio{real_audio} {}
	virtual ~Backend() = default;
};

} // namespace Szim

#endif // _VIY6OUAOIN4378G135Y_
