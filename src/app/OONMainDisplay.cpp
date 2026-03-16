#include "OONMainDisplay.hpp"
#include "OONConfig.hpp"

#include "Szim/diag/Log.hpp"
#include <cassert>

using namespace Szim::View;

namespace OON {

/*
//----------------------------------------------------------------------------
OONMainDisplay::OONMainDisplay(Config cfg) : cfg_(cfg)
{
	reset(); // Calc. initial state
}
*/

//----------------------------------------------------------------------------
OONMainDisplay::OONMainDisplay(OONViewConfig cfg, OONApp& app)
	: ScreenView(cfg, oon_default_camera_)
	, app_(app)
	, oon_default_camera_({.base_scale = OONConfig::DEFAULT_ZOOM})
{
//	reset(); // Calc. initial state
	LOGD <<	"Camera pointer (_camera): " << camera_;
}

//----------------------------------------------------------------------------
const OONAvatar& OONMainDisplay::avatar(size_t ndx) const
{
//!!	assert(avatars_.size());
if (!avatars_.size()) { static OONAvatar dummy({});
LOGN <<"- Hey, avatar loading must happen before other View ops.!";
return dummy; }

	return *avatars_[ndx > avatars_.size() - 1
	                     ? avatars_.size() - 1 : ndx];
}


/*
void OONMainDisplay::reset(const Config* recfg)
{
	if (recfg) cfg_ = *recfg;

	resize(cfg_.width, cfg_.height);
}

void OONMainDisplay::reset(Config&& recfg) { reset(&recfg); }

void OONMainDisplay::resize(unsigned width, unsigned height)
{
	cfg_.width  = width;
	cfg_.height = height;
}
*/

} // namespace OON
