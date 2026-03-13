#ifndef Y8GVVY7TC880X820KS272475BTBT7V
#define Y8GVVY7TC880X820KS272475BTBT7V

#include "app/model/vocab.hpp" // BasicNumberType

#include "Szim/Model/Physics/Real.hpp"

namespace OON::Model {

// Make sure the configured basic number type is used by each client of the model physics:
template <typename NumT> class Physics;
using Phys = Szim::Model::Physics<OON::Model::BasicNumberType>; //!! The same should be done for Math, too!

} // namespace OON::Model

#endif // Y8GVVY7TC880X820KS272475BTBT7V
