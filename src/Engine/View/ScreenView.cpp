#include "ScreenView.hpp"

//#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"

namespace Szim::View {

/*
ScreenView::ScreenView(Config cfg) : _cfg(cfg)
{
	reset(); // Calc. initial state
}
*/

ScreenView::ScreenView(Config cfg, Camera& cam) : _cfg(cfg), _camera(&cam)
{
	reset(); // Calc. initial state
}


void ScreenView::reset(const Config* recfg/* = nullptr*/)
{
	if (recfg) _cfg = *recfg;

	resize(_cfg.width, _cfg.height);
}

void ScreenView::reset(Config&& recfg) { reset(&recfg); }

void ScreenView::resize(unsigned width, unsigned height)
{
	_cfg.width  = width;
	_cfg.height = height;
}


void ScreenView::attach(Camera& camera)
{
	_camera = &camera;
}

} // namespace Szim::View
