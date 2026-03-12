#ifndef OEDM04785YTB0872ND45076Y20745Y68CN74
#define OEDM04785YTB0872ND45076Y20745Y68CN74


#include "app/model/vocab.hpp"

#include "Szim/Model/Entity.hpp" // Szim::Entity<Cfg>

#include "Physics.hpp"
#include "Thruster.hpp"

#include <iosfwd> // for save/load

#include "Szim/diag/Log.hpp"


namespace OON::Model {

//============================================================================
	struct EntityConfig
	{
		using CoordT = Phys::NumType;
	};

	struct Entity : public Szim::Model::Entity<EntityConfig>
		//!! : public Serializable //! No: this would kill the C++ designated init syntax! :-/
	                                       //! Also old-school; template-/concept-based approaches are superior.
	                                       //! Keep it trivially_copyable (not POD: they can't have ctors!) for easy loading!
	{
		using NumType = Phys::NumType;

		//!!ObjConfig cfg; // basically the obj. type

		static constexpr auto Unlimited = -1.f;  // For various quantities (usually float; having
		                                         // this also float mutes some MSVC conv. warnings).

		struct {
			bool gravity_immunity = false;
			bool free_color = false; // T doesn't affect color
		} superpower;

		// Presets:
		Phys::Time lifetime = Unlimited; // how many s to Event::Decay; < 0 means stable end state that can't decay (any further)
		Phys::Length r = 0; // Calculated from mass and density
		Phys::Density density = Phys::DENSITY_ROCK / 2; //!!low-density objects should look like Swiss cheese! ;)
		Phys::Pos2  p{0, 0};
		Phys::Velo2 v{0, 0};
		Phys::Temperature T = 0; // affected by various events; represented by color

		// These are used by the engine (e.g. is_entity_at_viewpos)
		auto position() const { return p; }
		auto hit_radius(Szim::Model::Interaction::Any) const { return r; }
		//auto hit_radius(Szim::Model::Interaction::UI) const { return r; }

		// Preset/recomputed:
		uint32_t color = 0; // if left 0, it'll be recalculated from T (if not 0)
			// RGB (Not containing an alpha byte (at LSB), so NOT compatible with the SFML Color ctors!
			// The reason is easier add_body() calls here.)

		Phys::Mass mass;

		//!! Ugly hack to start generalizing object compositions & to allow the world
		//!! to calc. propulsion without consulting the controller. (I mean this is still
		//!! less ugly than these being part of the game app (controller) itself! :) )
		//!! Should be handled later indirectly (and uniformly) via object configurations
		//!! (which would basically be a flexible type system).
		//!!
		// For entities with actually functioning thrusters, these will be changed
		// from `unset` (to real numbers, i.e. 0).
		Thruster thrust_up    { Math::unset<Thruster::NumType>() };
		Thruster thrust_down  { Math::unset<Thruster::NumType>() };
		Thruster thrust_left  { Math::unset<Thruster::NumType>() };
		Thruster thrust_right { Math::unset<Thruster::NumType>() };
		//!!ALSO: REPLACE THIS HARDCODED COMPONENT WITH A GENERIC (dynamically built)
		//!! `Structure` COMPONENT + ("OPTONAL") TYPE INFO!
		//!! ("OPTIONAL" 'coz the structure itself *IS* the type info, it's just cumbersome to work with.)

		//! Alas, can't do this with designated inits: Entity() : mass(powf(r, 3) * density) {} :-(
		//! So... (see e.g. add_body()):
		void recalc();
		bool can_expire() const noexcept { return lifetime > 0; }
		void terminate()        noexcept { lifetime = 0; } // Currently the best fit...
		bool terminated() const noexcept { return lifetime == 0; }
		void on_event(Event e, ...); //! Alas, can't be virtual: that would kill the C++ desig. init. syntax (among other things)! :-(

		// Ops.:
		bool has_thruster() const noexcept { return !Math::is_unset(thrust_up.thrust_level()); }
		void add_thrusters() noexcept { // Umm...: ;)
			thrust_up.thrust_level(0);
			thrust_down.thrust_level(0);
			thrust_left.thrust_level(0);
			thrust_right.thrust_level(0);
		}
		bool is_player() const noexcept { return has_thruster(); } //!! ;)

		bool        save(std::ostream&);
		static bool load(std::istream&, Entity* result = nullptr); // Verifies only (comparing to *this) if null
	};


} // namespace OON::Model


#endif // OEDM04785YTB0872ND45076Y20745Y68CN74
