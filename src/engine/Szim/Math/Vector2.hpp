#ifndef _SDMF7945YH78DFYRIUNT670C83Y7RFTYU9C5V86BMR_
#define _SDMF7945YH78DFYRIUNT670C83Y7RFTYU9C5V86BMR_


//#include "sz/math/vec/dc/vector.hpp"
#include "sz/math/vec/dc/adapter.hpp"
#include "sz/math/vec/adapter/SFML.hpp"


namespace Math
{
//	using namespace VEC_NAMESPACE;

//----------------------------------------------------------------------------
// "Dictionary" type aliases...
//----------------------------------------------------------------------------
	using VEC_NAMESPACE::Scalar;

//	template <unsigned Dim = 2, Scalar NumT = float>
//	using V2 = VEC_NAMESPACE::Vector<Dim, NumT>; //, adapter::VectorAdapter>;

	template <Scalar NumT = float>
	using V2 = VEC_NAMESPACE::VEC_DIRECT_COORD_SUBNAMESPACE::Vector<2, NumT, VEC_NAMESPACE::adapter::SFML>;

	using V2f = V2<float>;
	using V2d = V2<double>;
	using V2i = V2<int>;
	using V2u = V2<unsigned>;

} // namespace Math


#endif // _SDMF7945YH78DFYRIUNT670C83Y7RFTYU9C5V86BMR_
