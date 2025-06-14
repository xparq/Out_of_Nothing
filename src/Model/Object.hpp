#ifndef _DM04785YTB0872ND45076Y20745Y68CN74_
#define _DM04785YTB0872ND45076Y20745Y68CN74_


#include "Thruster.hpp" //!! This must come before World.hpp, in this convoluted setup...

#include "World.hpp" //!! Wow, have I no fear?! ;)
                     //!! This hack makes #include "Object.hpp" work transparently...
                     //!! (Wouldn't have this scandal if World::Body could be defined here, separately.)


namespace Model {

//------------------------------------------------------------------------
//!! Dear C++, wouldn't it be nice if World::Body could be defined here, separately from World.hpp?
//!! class World::Body { ... };
//------------------------------------------------------------------------

} // namespace Model


#endif // _DM04785YTB0872ND45076Y20745Y68CN74_
