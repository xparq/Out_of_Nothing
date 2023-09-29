#ifndef _SFML_shim_hpp_
#define _SFML_shim_hpp_


#include "Model/SFML/Vector2.hpp"

template <class T>
sf::Vector2<T> to_sfVector2(sfml::Vector2<T> v) { return sf::Vector2<T>(v.x, v.y); }

/*!!
#include "Model/SFML/Vector3.hpp"
template <class T>
sf::Vector3<T> to_sfVector3(sfml::Vector3<T> v) { return sf::Vector3<T>(v.x, v.y, v.z); }

	//!!?? No Vector3 in SFML any more?!
	//!! Should change that wicked sfml:: namespace anyway to something clearly incompatible!...

!!*/


#endif//_SFML_shim_hpp_
