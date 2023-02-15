#ifndef __OON__
#define __OON__

#include "SimApp.hpp"

//============================================================================
class OON : public SimApp
{
public:
	void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...) override;

	OON(const OON&) = delete;
public:
	OON() {}
};

#endif // __OON__
