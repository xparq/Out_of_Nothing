//!! Fix Vector2f using std::is_floating_point_v or atan etc. without including these! :-o
//!!?? Why has this been masked so far?!?
#include <cmath>
#include <type_traits>

#include "ViewPort.hpp"

using namespace Szim;
using namespace View;

ViewPort::ViewPort()
{
	//!! These might need to be adjusted dynamically though!
	_edge_x_min = -width/2;
	_edge_x_max =  width/2;
	_edge_y_min = -height/2;
	_edge_y_max =  height/2;
}


void ViewPort::confine(Math::Vector2f world_pos)
{
	auto vpos = world_to_view_coord(world_pos);
	if      (vpos.x <  _edge_x_min) offset.x -= _edge_x_min - vpos.x + 2;
	else if (vpos.x >= _edge_x_max) offset.x += vpos.x - _edge_x_max + 2;
	if      (vpos.y <  _edge_y_min) offset.y -= _edge_y_min - vpos.y + 2;
	else if (vpos.y >= _edge_y_max) offset.y += vpos.y - _edge_y_max + 2;
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