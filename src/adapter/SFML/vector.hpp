#ifndef _SFML_vector_adapter_hpp_
#define _SFML_vector_adapter_hpp_

#include "Model/Math/Vector2.hpp"
#include "SFML/System/Vector2.hpp"
template <class T>
sf::Vector2<T> to_sfVector2(Math::Vector2<T> v) { return sf::Vector2<T>(v.x, v.y); }

#include "Model/Math/Vector3.hpp"
#include "SFML/System/Vector3.hpp"
template <class T>
sf::Vector3<T> to_sfVector3(Math::Vector3<T> v) { return sf::Vector3<T>(v.x, v.y, v.z); }

#endif//_SFML_vector_adapter_hpp_
