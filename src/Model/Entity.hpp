#ifndef _DM04785YTB0872ND45076Y20745Y68CN74_
#define _DM04785YTB0872ND45076Y20745Y68CN74_


#include "Engine/Metamodel.hpp"
#include "Physics.hpp"
#include "Thruster.hpp"
#include "Engine/Config.hpp"

#include <iosfwd> // for save/load


namespace Model {

	//--------------------------------------------------------------------
	struct Entity //!! : public Serializable //! No: this would kill the C++ designated init syntax! :-/
	                                       //! Also old-school; template-/concept-based approaches are superior.
	                                       //! Keep it trivially_copyable (not POD: they can't have ctors!) for easy loading!
	{
		using NumType = Phys::NumType;

		// Placeholder/sentinel ID for "no entity":
		//!! Clean this hackfest up (see them also in Metamodel.hpp!):
		constexpr static EntityID NONE = NO_ENTITY;
		constexpr static auto     Unlimited = Model::UNLIMITED;

		//!!ObjConfig cfg; // basically the obj. type


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
		//!!BTW, thrust should be axial anyway, so these 4 should be just 2:
		//!!ALSO: REPLACE WITH A GENERIC (dynamically built) Structure COMPONENT + ("OPTONAL") TYPE INFO!
		//!!      ("OPTIONAL" 'coz the structure itself *IS* the type info, it's just cumbersome to work with!)
		Thruster thrust_up    { Math::MyNaN<NumType> }; // Gets repalced by "real" numbers for objects with actually functioning thrusters.
		Thruster thrust_down  { Math::MyNaN<NumType> };
		Thruster thrust_left  { Math::MyNaN<NumType> };
		Thruster thrust_right { Math::MyNaN<NumType> };

		//! Alas, can't do this with designated inits: Entity() : mass(powf(r, 3) * density) {} :-(
		//! So... (see e.g. add_body()):
		void recalc();
		bool can_expire() const noexcept { return lifetime > 0; }
		void terminate()  noexcept { lifetime = 0; } // Currently the best fit...
		bool terminated() const noexcept { return lifetime == 0; }
		void on_event(Event e, ...); //! Alas, can't be virtual: that would kill the C++ init. list syntax! :-o :-/

		// Ops.:
		bool has_thruster() { return thrust_up.thrust_level() != Math::MyNaN<float>; } //!! Ugh!... :-o :)
		void add_thrusters() { // Umm...: ;)
			thrust_up.thrust_level(0);
			thrust_down.thrust_level(0);
			thrust_left.thrust_level(0);
			thrust_right.thrust_level(0);
		}
		bool is_player() { return has_thruster(); } //!! ;)

		bool        save(std::ostream&);
		static bool load(std::istream&, Entity* result = nullptr); // Verifies only (comparing to *this) if null
	};


} // namespace Model


#endif // _DM04785YTB0872ND45076Y20745Y68CN74_
