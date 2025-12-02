#ifndef _MECV8V7I4G978D4F7384T7984CV9536G98U_
#define _MECV8V7I4G978D4F7384T7984CV9536G98U_

#include "Meta.hpp"

namespace Szim {
namespace Model::Core {

//!! Meta-entity stuff, like classes/prototypes/archetypes, even vtables etc...
//!! Basically a run-time class system. :-/ So, keep an eye on C++26/29 reflection, too!

struct Entity
{
	// Placeholder/sentinel ID for "no entity":
	//!! Clean this hackfest up (see them also in Metamodel.hpp!):
	constexpr static EntityID NONE      = Szim::Model::NO_ENTITY;
	constexpr static auto     Unlimited = Szim::Model::UNLIMITED;

};

} // namespace Model::Core
} // namespace Szim

#endif // _MECV8V7I4G978D4F7384T7984CV9536G98U_
