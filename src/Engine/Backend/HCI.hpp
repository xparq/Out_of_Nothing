#ifndef _37H6FG50V6BC2376T24MYHM022CJU9I45_
#define _37H6FG50V6BC2376T24MYHM022CJU9I45_

#include <string> //!! Sigh... I really shouldn't be introducing this dependency here! :-/

namespace Szim {

class HCI
{
public:
	struct Window {
		struct Config {
			unsigned width = 0, height = 0;
			std::string title; // Unused if fullscreen.
			bool fullscreen = false;
			bool headless = false;
		};

		mutable Config cfg; // "Reconfigured" by every create!

		Window(const Config& requested_cfg) : cfg(requested_cfg) {}
		virtual ~Window() = default;
//!! This can't help, as it would be called from the ctor, which can't dispatch virtuals:
//!!protected:
//!!		virtual void create(const Config&) {}
//!!		Window(const Config& requested_cfg) create(requested_cfg); }
	};

/*!!?? The specific subclass ctor will get the SimApp cfg, so it can store a ref. to that!
	struct Config //!! : LightGenericExtensibleConfig (possibly abstract),
	              //!!   that's also the base of Szim::Config! :-o
	{
		Window::Config window; //!! But this would make inheriting a nightmare! :-/
	}
	const Config cfg;
??!!*/
	public: virtual Window& main_window() { static Window dummy({}); return dummy; }
	public: virtual void switch_fullscreen(bool fullscreen [[maybe_unused]]) { }
	public: virtual void set_frame_rate_limit(unsigned fps [[maybe_unused]]) { }
	public: unsigned get_frame_rate_limit() { return _last_fps_limit; }

	virtual ~HCI() = default;

protected:
	unsigned _last_fps_limit = 0;
};

} // namespace Szim

#endif // _37H6FG50V6BC2376T24MYHM022CJU9I45_
