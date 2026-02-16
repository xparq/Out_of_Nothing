#ifndef _MWMV4HU8VM97CCF2M4Y6879YMMMMMMMMMMCM45YNVB7986_
#define _MWMV4HU8VM97CCF2M4Y6879YMMMMMMMMMMCM45YNVB7986_


#include "Szim/Model/vocab.hpp"
#include "app/Model/cfg.hpp"

namespace OON::Model
{
	using EntityID = Szim::Model::EntityHandle;

	using BasicNumberType = Cfg::BasicNumberType;
//!!??	using BasicNumberType = Szim::Model::BasicNumberType<Cfg::BasicNumberType>; //!! Make it selectable here by the app, with a Szim default!

	using DeltaT   = Szim::Model::TimeSpan;

	using Event    = Szim::Model::Event;

	struct Entity; //!! Just fw-decl. instead of including model/Entity.hpp up-front...
	               //!!?? How (un)stable is this here, actually?
}


#endif // _MWMV4HU8VM97CCF2M4Y6879YMMMMMMMMMMCM45YNVB7986_
