#ifndef _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_
#define _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_

#include "Engine/Model.hpp"
//!!#include "Model/Object.hpp" //!! Not really needed yet (also includes loads of crap.)
#include "Model/Physics.hpp" //!! Model should be split into Engine/ generic & app/ (or ext/!) specific one!
#include "Model/Math/Vector2.hpp"

namespace Szim { class SimApp; }

namespace Model {

class Emitter //!!?? : public Entity
{
public:
	using NumT = Phys::NumType;
	//----------------------------------------------------------------------------
	struct Config
	{
		Math::Vector2<NumT> eject_velocity{}; // Relative to the emitter's v
		Math::Vector2<NumT> eject_offset{};   // Relative to the emitter's origin
		NumT v_factor = 0.1f; //!! May be redundant with eject_velocity now!
		NumT offset_factor = 0.2f;
		float particle_lifetime = Model::Unlimited; //!! Rename to sg. less vague!
		bool  create_mass = true;
		NumT particle_density = Phys::DENSITY_ROCK * 0.001f;
		Math::Vector2<NumT> position_divergence = {5.f, 5.f}; // Scaled by the emitter's radius
		NumT velocity_divergence = 1.f; //!! Just an exp. "randomness factor" for now!...
		NumT particle_mass_min{};
		NumT particle_mass_max{};
		uint32_t color = 0x706080; // 0xRRGGBB
	};

	//----------------------------------------------------------------------------
	Emitter(const Config& cfg, Szim::SimApp& app);
	virtual ~Emitter() = default;

	void emit_particles(size_t emitter_ndx, unsigned n = 10, Math::Vector2<NumT> nozzles[] = nullptr);
		// 'nozzles' must have n elements if not null, each relative to the
		// origin of the emitter, and normalized to a [-1, 1] bounding range!

protected:
	Szim::SimApp& app;
public:
	Config        cfg;
};

} // namespace Model

#endif // _MSD0F8743567836U7KRINUC87N3OE6B5UOYRFIUEG_
