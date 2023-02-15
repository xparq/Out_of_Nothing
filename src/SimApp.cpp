#include "SimApp.hpp"

bool SimApp::collide_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2, float distance)
{w, obj1, obj2, distance;
	//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
	//!!...body->p -= ds...;
	return false;
}

bool SimApp::touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2)
{w, obj1, obj2;
	return false;
}

// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
//!!The model should also pass the physical property/condition ("event type") that made it think these may interact!
//!!A self-documenting alternative would be calling a matching function for each known such event,
//!!but that might be prohibitively expensive in that tight loop, especiall if most callbacks would
//!!just do nothing.
//!!NOTE: This will anyway change to the objects themselves being notified (not the game "superclass")!
void SimApp::interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...)
{w, event, obj1, obj2;
	//!!?? body->interact(other_body) and then also, per Newton, other_body->interact(body)?!
}
