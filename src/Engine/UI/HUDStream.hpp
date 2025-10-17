#ifndef _HUSM09827N4536R87B4M9O8HG_
#define _HUSM09827N4536R87B4M9O8HG_

//!!?? #include "Engine/Config.hpp" //!! Move to szlib (#398)

#include "myco/widget/StreamBox.hpp"

namespace UI {

//----------------------------------------------------------------------------
class HUDStream : public myco::StreamBox
{
public:
	using myco::StreamBox::StreamBox; // Take the ctors...

	//!! Non-virt. adapter (public access bridge, etc.) until the HUDs
	//!! get properly managed by the UI, instead of manually in OONApp!...
	void draw(const myco::gfx::RenderContext& ctx) const { myco::StreamBox::draw(ctx); }
};

}; // namespace UI

#endif // _HUSM09827N4536R87B4M9O8HG_
