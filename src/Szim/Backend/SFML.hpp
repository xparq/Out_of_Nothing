#ifndef _IWEURYDIUTEWYBVUCQVXT34V76_
#define _IWEURYDIUTEWYBVUCQVXT34V76_

#include "cfg.h"
#include "Backend.hpp"

#include "Backend/SFML/Audio_SFML.hpp"

#include <iostream>

namespace Szim {

class Backend_SFML : public Backend
{
public:
	Backend_SFML() : Backend{Audio_SFML::create()}
	{ std::cerr << "Hi. Initializing the SFML backend.\n"; }

	static Backend& create();

	void SomeAPI() override {}
}; // class Backend_SFML

Backend& Backend_SFML::create()
{
	static Backend_SFML backend{};
	return backend;
}

} // namespace Szim
#endif // _IWEURYDIUTEWYBVUCQVXT34V76_
