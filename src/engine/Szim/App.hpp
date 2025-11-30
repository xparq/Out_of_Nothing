#ifndef _SADFM987HUYY09786UHYTIUHGJM9587CUV_
#define _SADFM987HUYY09786UHYTIUHGJM9587CUV_

#include "SimApp.hpp"
#include <type_traits>

namespace Szim {

template <typename AppTraits>
class App : public SimApp
{
public:
	// Lift the base ctors:
	//!! BUT... NO! Those WON'T BE ABLE to also initialize any app-typed data
	//!! managed here!
	//!! SO, ONLY KEEP THIS FOR THE MIGRATION TRANSIENT:
	using SimApp::SimApp;

#if 0
	// "Internalize" the app types...
	using WorldT  = AppTraits::WorldT;
	using EntityT = AppTraits::EntityT;
	//!!using PlayerT = typename AppTraits::Player;
	//!!...

	//!!static_assert(std::derived_from<WorldT, Szim::Model::World>);


protected:
	std::unique_ptr<WorldT> create_world() override
	{
		return std::make_unique<WorldT>();
	}

public:
/*!! OLD, from SimApp:
	      Model::World& world();
	const Model::World& world() const;
	const Model::World& const_world(); // Explicit const World& of non-const SimApp (to spare a cast)
	void set_world(std::unique_ptr<Model::World>);
!!*/
	WorldT& world()
	{
		return *static_cast<WorldT*>(SimApp::world_.get());
	}

/*!!
	// 1. RESTORE THE WORLD API
	// The App sees "GetEntities()" again, typed correctly.
	SpanLite<EntityT> entities() {
		return { (EntityT*)SimApp::entity_data(0), SimApp::entity_count() };
	}

	EntityT& entity(EntityID id) { return entities()[id]; }

	// 2. RESTORE ADD_ENTITY
	EntityID add_entity(EntityT&& temp) {
		EntityID id = AllocEntitySlot();
		// Placement new into the raw blob
		new (&entity(id)) EntityT(std::move(temp));
		return id;
	}

	// 3. RESTORE PLAYERS
	// The vectors live here now, where the type is known.
	std::vector<PlayerT> players;

	// 4. IMPLEMENT INFRASTRUCTURE
	void SerializeWorld(Stream& s) override {
		// Write m_entityBlob directly...
	}
!!*/
protected:
	App() {
		SimApp::entity_size_ = sizeof(EntityT);
	}
#endif
};


} // namespace Szim

#endif // _SADFM987HUYY09786UHYTIUHGJM9587CUV_
