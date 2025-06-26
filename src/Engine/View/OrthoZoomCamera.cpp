#include "OrthoZoomCamera.hpp"

#include "sz/math/sign.hh"

//#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"

namespace Szim::View {

/*
OrthoZoomCamera::OrthoZoomCamera()
{
	reset(); // Calc. initial state
}
*/
OrthoZoomCamera::OrthoZoomCamera(Config cfg) :
	cfg(cfg)
{
	reset(); // Calc. initial state
}

void OrthoZoomCamera::reset(const Config* recfg/* = nullptr*/)
{
	if (recfg) cfg = *recfg;

//std::cerr << "DBG> "<<__FUNCTION__<<": width="<<cfg.width<<", height="<<cfg.height<<"\n";

	// Reset all the derived state, too...
	reset_zoom();
	view_offset = {0, 0};
	focus_offset = {0, 0};

	resize_view(cfg.width, cfg.height);
}


void OrthoZoomCamera::resize_view(float width, float height)
{
	//!! Adjust the offsets only if the resize would shift them off-screen!
	//!! But even then, such a forced `track()` may be undesired in some cases!
	//!!new_offset = ...
	//!!focus_offset = new_offset - offset; //!! Or what exactly is this relative to? I forgot... :)

	cfg.width  = width;
	cfg.height = height;

	_edge_x_min = -cfg.width/2;
	_edge_x_max =  cfg.width/2;
	_edge_y_min = -cfg.height/2;
	_edge_y_max =  cfg.height/2;

//std::cerr << "DBG> "<<__FUNCTION__<<": new size: width="<<cfg.width<<", height="<<cfg.height<<"\n";
}


void OrthoZoomCamera::reset(Config&& recfg) { reset(&recfg); } // ...it should live long enough for this, right? ;)

void OrthoZoomCamera::reset_zoom(float trim/* = 1*/)
{
	_scale = cfg.base_scale * trim;
}


void OrthoZoomCamera::look_at(WorldPos world_pos) //override
{
	view_offset = world_pos * _scale;
	set_focus_offset({0, 0});
}


bool OrthoZoomCamera::track(WorldPos world_pos, float margin, float throwback)
{
	auto vpos = world_to_view_coord(world_pos);
	bool out_of_view = false;

	if      (vpos.x <  _edge_x_min + margin) { out_of_view = true; view_offset.x -= _edge_x_min - vpos.x + margin + throwback; }
	else if (vpos.x >= _edge_x_max - margin) { out_of_view = true; view_offset.x += vpos.x - _edge_x_max + margin + throwback; }
	if      (vpos.y <  _edge_y_min + margin) { out_of_view = true; view_offset.y -= _edge_y_min - vpos.y + margin + throwback; }
	else if (vpos.y >= _edge_y_max - margin) { out_of_view = true; view_offset.y += vpos.y - _edge_y_max + margin + throwback; }

	return out_of_view;
}


void OrthoZoomCamera::zoom(float change_ratio)
{
	_scale *= change_ratio;

	// Compensate for zoom displacement when the focus is not centered
	pan_view((focus_offset + view_offset) * (change_ratio - 1));
		//!! 100% non-intuitive; I just solved the equations...
	        //!!?? What to make of that `world + view` coord type "mismatch"?! :-o
	        //!!?? And how would it translate to 3D?
}


Camera::ViewPos OrthoZoomCamera::grid_offset() const
{
//!!vpos = world_to_view_coord(view_offset);
//cerr <<"pan offset to view: "<< vpos.x <<", "<< vpos.y << '\n'; // BTW, non-zero if there's an off-center focus point

	auto vpos = view_offset;
	auto v = ViewPos{ // Easy-peasy, right?... ;)
		- (float(int(abs(vpos.x + _edge_x_max)) % int(cfg.width))  - _edge_x_max) * sz::sign(vpos.x + _edge_x_max),
		- (float(int(abs(vpos.y + _edge_y_max)) % int(cfg.height)) - _edge_y_max) * sz::sign(vpos.y + _edge_y_max)};
//cerr <<"- gridline pos. for offset ("<<view_offset.x<<", "<<view_offset.y<<"): "<< (v.x)<<", "<< (v.y) << '\n';
	return v;
}


Camera::ViewPos OrthoZoomCamera::screen_to_view_coord(int x, int y) const
{
	return {(float)x + _edge_x_min, _edge_y_max - (float)y};
//	auto v = V2f{(float)x + _edge_x_min, _edge_y_max - (float)y};
//cerr << "DBG> Camera coords. from screen pos ("<<x<<", "<<y<<"): "<<v.x <<", "<<v.y<<'\n';
//	return v;
}

} // namespace Szim::View
