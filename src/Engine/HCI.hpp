#ifndef _37H6FG50V6BC2376T24MYHM0_
#define _37H6FG50V6BC2376T24MYHM0_

namespace Szim {

struct HCI
{
	struct Window {};
	public: virtual Window& window() = 0;

	virtual ~HCI() = default;
};

} // namespace Szim

#endif // _37H6FG50V6BC2376T24MYHM0_
