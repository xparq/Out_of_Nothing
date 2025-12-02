#ifndef _WTYDFG984F98D46789F043VMU5V6MYU895R_
#define _WTYDFG984F98D46789F043VMU5V6MYU895R_


#include "Szim/Model/World.hpp"

#include <concepts>   // convertible_to
#include <memory>     // shared_ptr
#include <vector>


namespace Szim {
namespace Model {


//----------------------------------------------------------------------------
template <typename C>
concept WorldC = requires(C c)
{
	typename C::PropsT;
	// Require a static 'defaults' member of (or convertible to) PropsT:
	{ C::PropsT::defaults } -> std::convertible_to<typename C::PropsT>;	

	typename C::EntityT;
};


//----------------------------------------------------------------------------
template <WorldC Cfg>
class World : public Core::World
{
//----------------------------------------------------------------------------
// Data / World State... (!!-> Save/Load should cover this part (#76)!!)
//----------------------------------------------------------------------------
//protected: //!!NOT YET!
public:
	Cfg::PropsT props;

//!!protected: //!! Alas, .bodies is used directly in App<>!
	std::vector< std::shared_ptr<typename Cfg::EntityT> > bodies; //!! Can't just be made `atomic` by magic... (wouldn't even compile)

//----------------------------------------------------------------------------
// Misc...
//----------------------------------------------------------------------------
public:
	World(SimApp& _app)
		: Core::World::World(_app)
		, props(Cfg::PropsT::defaults)
	{}

	std::size_t entity_count() const override { return bodies.size(); }
};


} // namespace Model
} // namespace Szim


#endif // _WTYDFG984F98D46789F043VMU5V6MYU895R_
