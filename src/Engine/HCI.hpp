#ifndef _37H6FG50V6BC2376T24MYHM0_
#define _37H6FG50V6BC2376T24MYHM0_

//!!#include <string> //!! Sigh... I really shouldn't be introducing this dependency here! :-/

namespace Szim {

struct HCI
{
/*!!?? The specific subclass ctor will get the SimApp cfg, so it can store a ref. to that!
	struct Config //!! : LightGenericExtensibleConfig (possibly abstract),
	              //!!   that's also the base of Szim::Config! :-o
	{
		unsigned window_width, window_height; //!!?? LAST, OR DEFAULT?!
		std::string window_title;
	} cfg;
??!!*/
	struct Window {};
	public: virtual Window& window() = 0;
	public: virtual void switch_fullscreen(bool fullscreen) = 0;
	public: virtual void frame_rate_limit(unsigned fps) = 0;

	virtual ~HCI() = default;
};

} // namespace Szim

#endif // _37H6FG50V6BC2376T24MYHM0_
