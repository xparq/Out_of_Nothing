#include "Emitter.hpp"

#include "Engine/SimApp.hpp"


namespace Model {

Emitter::Emitter(const Config& emitter_cfg, Szim::SimApp& app)
	:
	cfg(emitter_cfg),
	app(app) //!! Sigh... Eliminate!
{}


//----------------------------------------------------------------------------
//!! - Move to SimApp!
//!! - Decouple from the entity() query: pass it the object, not the index!
//!!   (Only that `resize_shape(emitter_ndx, emitter.r/emitter_old_r);` uses it!
//!!   Could be done by callers, or even be a follow-up callback, if necessary.)
//!! - It still calls add_entity() (so still can't be a free function (or class)),
//!!   but that really could be a callback than...
void Emitter::emit_particles(size_t emitter_ndx, size_t n, Math::Vector2<NumT> nozzles[])
{
	auto& emitter = app.entity(emitter_ndx); // Not const: will deplete!
		//!! Also take care of other threads possibly deleting the emitter later on! :-o

//if (!cfg.create_mass) cerr <<"DBG> emitter.mass BEFORE burst: "<< emitter.mass <<'\n';

	auto p_range = cfg.position_divergence * emitter.r;
	auto p_offset = cfg.eject_offset;
	p_offset += // Also add a portion proportional to the emitter's velocity:
		(emitter.v == Math::Vector2<NumT>()) ? //! SFML-DEBUG asserts this, so must be prevented... :-/
			  Math::Vector2<NumT>()
			: emitter.v.normalized() * emitter.r * cfg.offset_factor;
		//!! Also needs a non-linearity (decoupling) factor so higher v can affect it less!

	auto v_range = emitter.r * cfg.velocity_divergence; //!! Ugh... by magic, right? :-o :-/

	auto emitter_old_r = emitter.r;

	for (int i = 0; i < n; ++i) {
		auto particle_mass = cfg.particle_mass_min + (cfg.particle_mass_max - cfg.particle_mass_min) * float(rand())/RAND_MAX;

		if (!cfg.create_mass && emitter.mass < particle_mass) {
//cerr << "- Not enough mass to emit particle!\n";
			continue;
		}
//cerr <<"DBG> density: "<< cfg.particle_density <<'\n';
//cerr <<"DBG>   ==?  : "<< Phys::DENSITY_ROCK * 0.0000000123f <<'\n';

		Math::Vector2<NumT> p = { (rand() * p_range.x) / RAND_MAX - p_range.x/2 + emitter.p.x + p_offset.x,
		                          (rand() * p_range.y) / RAND_MAX - p_range.y/2 + emitter.p.y + p_offset.y };
		                          //!!...Jesus, these "hamfixted" pseudo Δt "factors"...
		if (nozzles) p += nozzles[i] * emitter.r; // Scale to its "bounding sphere"...

		[[maybe_unused]] auto pndx = app.add_entity({ //!! Refact. to only use World::add_body directly!
			.lifetime = cfg.particle_lifetime,
			.density = cfg.particle_density,
			.p = p,
			.v = { (rand() * v_range) / RAND_MAX - v_range/2 + emitter.v.x * cfg.v_factor + cfg.eject_velocity.x,
			       (rand() * v_range) / RAND_MAX - v_range/2 + emitter.v.y * cfg.v_factor + cfg.eject_velocity.y },
			.color = cfg.color,
			.mass = particle_mass,
		});
//cerr <<"DBG> particle.r: "<< entity(pndx).r <<'\n';

//cerr <<"DBG> emitter v:  "<< emitter.v.x <<", "<< emitter.v.y <<'\n';
//cerr <<"     - eject Δv: "<< cfg.eject_velocity.x <<", "<< cfg.eject_velocity.y <<'\n';
//cerr <<"     - part. v:  "<< entity(pndx).v.x <<", "<< entity(entity_count()-1).v.y <<'\n';

		if (!cfg.create_mass) {
			emitter.mass -= particle_mass;
//cerr <<"DBG> Decreasing emitter.mass by: "<< particle_mass <<'\n';
		}
	}

	if (!cfg.create_mass) {
		assert(emitter.mass >= 0); // See the actual run-time check above!
//cerr <<"DBG> emitter.r before recalc: "<< emitter.r <<'\n';
		emitter.recalc();
//cerr <<"DBG> emitter.r after recalc: "<< emitter.r <<'\n';
		app.resize_shape(emitter_ndx, float(emitter.r/emitter_old_r));
//cerr <<"DBG> emitter.mass AFTER burst: "<< emitter.mass <<'\n';
	}
} // emit_particles

} // namespace Model
