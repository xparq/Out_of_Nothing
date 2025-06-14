#ifndef _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_
#define _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_

#include "Engine/Metamodel.hpp"
//!!#include "Model/Object.hpp" //!! Not really needed yet (also includes loads of crap.)
#include "Model/Physics.hpp" //!! Model should be split into Engine/ generic & app/ (or ext/!) specific one!

namespace Szim { class SimApp; }

namespace Model {

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
		float particle_lifetime = Model::Unlimited; //!! Rename to sg. less vague!
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

	void emit_particles(unsigned emitter_ndx, unsigned n = 10, Phys::Pos2 nozzles[] = nullptr);
		// 'nozzles' must have n elements if not null, each relative to the
		// origin of the emitter, and normalized to a [-1, 1] bounding range!

protected:
	Szim::SimApp& app;
public:
	Config        cfg;
};

} // namespace Model

#endif // _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_
