#ifndef _ETXMD03FUIGGIYFUG709F845G7VC5V_
#define _ETXMD03FUIGGIYFUG709F845G7VC5V_


#include "Szim/Model/Entity.hpp"


namespace Szim {
namespace Model {


//----------------------------------------------------------------------------
template <typename C>
concept EntityC = requires(C c)
{
	c;
};


//----------------------------------------------------------------------------
template <EntityC Cfg>
class Entity : public Core::Entity
{
public:

};


} // namespace Model
} // namespace Szim


#endif // _ETXMD03FUIGGIYFUG709F845G7VC5V_
