//!! Fix Vector2f using std::is_floating_point_v or atan etc. without including these! :-o
//!!?? Why has this been masked so far?!?
#include <cmath>
#include <type_traits>

#include "ViewPort.hpp"

using namespace Szim;
using namespace View;

ViewPort::ViewPort(Config cfg) :
	cfg(cfg)
{
	reset();
}

void ViewPort::reset()
{
	scale = cfg.base_scale;

	//!! These might need to be adjusted dynamically though!
	//!! Also, use .cfg!!! :-o
	_edge_x_min = -width/2;
	_edge_x_max =  width/2;
	_edge_y_min = -height/2;
	_edge_y_max =  height/2;
}


bool ViewPort::confine(Math::Vector2f world_pos, float margin, float throwback)
{
	auto vpos = world_to_view_coord(world_pos);
	bool out_of_view = false;

	if      (vpos.x <  _edge_x_min + margin) { out_of_view = true; offset.x -= _edge_x_min - vpos.x + margin + throwback; }
	else if (vpos.x >= _edge_x_max - margin) { out_of_view = true; offset.x += vpos.x - _edge_x_max + margin + throwback; }
	if      (vpos.y <  _edge_y_min + margin) { out_of_view = true; offset.y -= _edge_y_min - vpos.y + margin + throwback; }
	else if (vpos.y >= _edge_y_max - margin) { out_of_view = true; offset.y += vpos.y - _edge_y_max + margin + throwback; }

	return out_of_view;
}


void ViewPort::zoom(float change_ratio)
{
	scale *= change_ratio;

	// Compensate for zoom displacement when the player object is not centered
	pan((focus_offset + offset) * (change_ratio - 1));
		//!! 100% non-intuitive; I just solved the equations...
	        //!!?? What to make of that `world + view` coord type "mismatch"?! :-o
	        //!!?? And how would it translate to 3D?
}