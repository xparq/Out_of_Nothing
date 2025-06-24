#include "OONMainDisplay.hpp"
#include "OONConfig.hpp"

#include "Engine/diag/Log.hpp"

using namespace Szim::View;

namespace OON {

/*
OONMainDisplay::OONMainDisplay(Config cfg) : _cfg(cfg)
{
	reset(); // Calc. initial state
}
*/

OONMainDisplay::OONMainDisplay(OONViewConfig cfg, OONApp& app)
	: ScreenView(cfg, _oon_default_camera)
	, _app(app)
	, _oon_default_camera({.base_scale = OONConfig::DEFAULT_ZOOM})
{
//	reset(); // Calc. initial state
	LOGD <<	"Camera pointer (_camera): " << _camera;
}

/*
void OONMainDisplay::reset(const Config* recfg)
{
	if (recfg) _cfg = *recfg;

	resize(_cfg.width, _cfg.height);
}

void OONMainDisplay::reset(Config&& recfg) { reset(&recfg); }

void OONMainDisplay::resize(unsigned width, unsigned height)
{
	_cfg.width  = width;
	_cfg.height = height;
}
*/

} // namespace OON
