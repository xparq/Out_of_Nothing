#ifndef __RENDER_SFML__
#define __RENDER_SFML__

#include "cfg.h"

#include <SFML/Graphics.hpp>

#include <memory> // shared_ptr
#include <vector>
#include <iostream> // cerr
using namespace std;


class Engine_SFML;

//----------------------------------------------------------------------------
class Render_SFML // "View"
{
//----------------------------------------------------------------------------
public:
	static constexpr auto VIEW_WIDTH  = 800;
	static constexpr auto VIEW_HEIGHT = 600;

	static constexpr const auto ALPHA_ACTIVE = 255;
	static constexpr auto ALPHA_INACTIVE = 127;

// Input params
	uint8_t p_alpha;

// Internals:
//!!...not quite yet -- just allow access:
public:
	vector< shared_ptr<sf::Drawable> >      shapes_to_draw; // ::Shape would be way too restritive here
	//!!misnomer alert below! should be sg. like "body_images" (as they are not just any Transformables!!! -- these are linked to physical bodies!):
	vector< shared_ptr<sf::Transformable> > shapes_to_change; // ::Shape would be way too restritive here

// Ops
	void render_next_frame(const Engine_SFML& game); // can't keep it inline here: uses the Engine!

// Housekeeping
	Render_SFML()
	      :	p_alpha(ALPHA_ACTIVE)
	{
	}
};

#endif // __RENDER_SFML__
