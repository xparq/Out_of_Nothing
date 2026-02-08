#ifndef _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_
#define _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_

#include "app/model/vocab.hpp"
//#include "app/Model/Entity.hpp" //!! Not really needed yet (also includes loads of crap.)
//#include "Szim/Model/Entity.hpp"
#include "app/Model/Physics.hpp" //!! `Model` should be split into (generic) Szim/ & (spec.) app/ parts!
//!!#include "Szim/Model/Physics.hpp"
//!!	using Phys = Szim::Model::Phys;


namespace Szim { class SimApp; }

namespace OON::Model {

class Emitter //!!?? : public Entity
{
public:

	//!!C++ bullshit: Just `using namespace Phys` is not allowed in a class decl.
	using NumT = Phys::NumType;
	//----------------------------------------------------------------------------
	struct Config
	{
		Phys::Velo2 eject_velocity{}; // Relative to the emitter's v
		Phys::Pos2  eject_offset{};   // Relative to the emitter's origin
		NumT v_factor = 0.1f; //!! May be redundant with eject_velocity now!
		NumT offset_velo_factor = 0.2f; // 1/(m/s)
		float particle_lifetime = UNLIMITED;
		bool  create_mass = true;
		Phys::Density particle_density = Phys::DENSITY_ROCK * 0.001f;
		Phys::Pos2 position_divergence = {5.f, 5.f}; // Scaled by the emitter's radius
		NumT velocity_divergence = 1.f; //!! Just an exp. "randomness factor" for now!...
		Phys::Mass particle_mass_min{};
		Phys::Mass particle_mass_max{};
		uint32_t color = 0x706080; // 0xRRGGBB
	};

	//----------------------------------------------------------------------------
	Emitter(const Config& cfg, Szim::SimApp& app);
	virtual ~Emitter() = default;

	void emit_particles(EntityID emitter, unsigned n = 10, Phys::Pos2 nozzles[] = nullptr);
		// 'nozzles' must have n elements if not null, each relative to the
		// origin of the emitter, and normalized to a [-1, 1] bounding range!

protected:
	Szim::SimApp& app;
	//!!enum { UNLIMITED = Szim::Model::UNLIMITED }; //!! UNLIMITED is a float!... :-/
	static constexpr auto UNLIMITED = -1;

public:
	Config        cfg;
};

} // namespace OON::Model

#endif // _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_
