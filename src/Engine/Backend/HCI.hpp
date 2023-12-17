#ifndef _37H6FG50V6BC2376T24MYHM022CJU9I45_
#define _37H6FG50V6BC2376T24MYHM022CJU9I45_

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
	struct Window { unsigned width = 0, height = 0; };
	public: virtual Window& window() = 0;
	public: virtual void switch_fullscreen(bool fullscreen) = 0;
	public: virtual void set_frame_rate_limit(unsigned fps) = 0;
	public: unsigned get_frame_rate_limit() { return _last_fps_limit; }

	virtual ~HCI() = default;

protected:
	unsigned _last_fps_limit = 0;
};

} // namespace Szim

#endif // _37H6FG50V6BC2376T24MYHM022CJU9I45_
