#ifndef _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#define _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
#include "_build_cfg.h"

#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?
#include "Backend.hpp"
#include "SimAppConfig.hpp"
#include "SessionManager.hpp"
#include "Time.hpp"
#include "Avatar.hpp" // Fw-decl. is not enough for vector<Avatar>: namespace Szim { class Avatar; }
#include "Player.hpp" // Fw-decl. is not enough for vector<Player>: namespace Szim { class Player; }

//!!... The UI and IO etc. are gonna be tough to abstract...
//!!namespace sfw { class GUI; }
#include "sfw/GUI.hpp"//!! REMOVE FROM HERE! (After hiding it behind a ref., those
                      //!! (mostly?) client .cpps that use should include it individually!)
//!!#include "UI/HUD.hpp"
#include "UI/Input.hpp"
#include "Metamodel.hpp" //!! Just a reminder/placeholder yet...
#include "Model/World.hpp" //!! The generic Model parts should move to Metamodel, and the specifics should be included from the (specific) app!
//#include "View/ScreenView.hpp"
namespace Szim::View { class ScreenView; }

#include "sz/lang/.hh" // ON/OFF, AUTO_CONST, OUT
#include "sz/stat/counter.hh"
#include "sz/stat/rolling_average.hh"

//!!GCC still doesn't like modules:
//!!import Storage;

#include <atomic>
#include <format> // vformat
#include <string>
#include <string_view>
#include <vector>
#include <cassert>


namespace Szim {

//============================================================================
class SimApp // Universal Sim. App Base ("Engine Controller")
{
//----------------------------------------------------------------------------
// Config...
//----------------------------------------------------------------------------
protected:
	static constexpr auto DEFAULT_CFG_FILE = "default.cfg";

	// See also: SimAppConfig

//----------------------------------------------------------------------------
// API...
//----------------------------------------------------------------------------
public:
	//!!virtual... -> #348
	static auto show_cmdline_help(const Args& args, const char* description = nullptr) -> void;

	int run(); // Returns the intended exit code for the process (!0: error)
	           // Note: "exit codes" may not be applicable in every execution model!
	virtual void init(); // Call request_exit(exit_code) to mark it "aborted".
	                     // No need to call the "upstream" init() from an override.
	virtual void done(); // Optional cleanup; will not be called if init() was aborted.
	                     // No need to call the "upstream" done() from an override.
	virtual void poll_controls() {}
	virtual bool perform_control_actions() { return false; } // false: no model changes

	virtual void init_world() { world().init(*this); } //!!TODO: Called by the default init(), before the 1st update_world().
	virtual void update_world(Time::Seconds Δt) { world().update(Δt, *this); }

	unsigned fps_throttling(unsigned fps = unsigned(-1));
		// Set or query the FPS limit (the default -1 means query)
		//!! std::optional couldn't help eliminate it altogether

	void fps_throttling(bool newstate);
		// Enable/disable configured FPS limit

	void request_exit(int exit_code = 0); // (The code set by the last request_exit(x) will win.)
	bool terminated() const { return _terminated; }
	int  exit_code() const { return _exit_code; } // The code set by the last request_exit(x), or 0
	//!!TBD: Setting it directly without request_exit(x):
	//int|void exit_code(int exit_code) { return _exit_code = exit_code; }

	void pause(bool newstate = true);
	auto paused() const { return time.paused; }
	bool toggle_pause(); // Returns the new state
	virtual void pause_hook(bool /*newstate*/) {} // Pausing might need followup actions in the mechanics

	void toggle_fullscreen();

	bool toggle_fixed_model_dt(); // Returns the new state


	float session_time() const { return time.real_session_time; }
	virtual void time_step(int /*steps*/) {} // Negative means stepping backward!

	      Model::World& world();
	const Model::World& world() const;
	const Model::World& const_world(); // Explicit const World& of non-const SimApp (to spare a cast)
	void set_world(const Model::World&);

	// Visualizing, rendering...
	//!! Tentative! This "main_view" name is just a placeholder/reminder
	//!! that there can be any kinds of (multiple) views!
	      View::ScreenView& main_view()       { return _main_view; }
	const View::ScreenView& main_view() const { return _main_view; }

	// Session save/load...
	enum SaveOpt { UseDefaults = -1, Raw = 0, Compress = 1 };
	virtual bool save_snapshot(const char* filename, SaveOpt flags = UseDefaults);
	virtual bool load_snapshot(const char* filename);
	bool quick_save_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	bool quick_load_snapshot(unsigned slot = 1); // See cfg.quick_snapshot_filename_pattern!
	template <typename... X> // This must be a template to support custom patterns + args:
	std::string snapshot_filename(size_t slot_ndx = 1,
		std::string_view pattern = SimAppConfig::DEFAULT_SNAPSHOT_FILE_PATTERN,
		const X... args)
	{
		return std::vformat(pattern, std::make_format_args(slot_ndx, args...));
	}

	// Entities...

	using Entity = Model::Entity;

	size_t entity_count() const { return world().bodies.size(); }
//!! ADD DEBUG-MODE BOUNDS-CHECKING FOR THESE!
	// Thread-safe, slower access:
	      Entity& entity(size_t index)       { return *world().bodies[index]; }
	const Entity& entity(size_t index) const { return *world().bodies[index]; }
	const Entity& const_entity(size_t index) { return *world().bodies[index]; }
	//!! This might be misguided, but keeping it as a reminder...
	// Unprotected, faster access (when already locked):
	      Entity& _entity(size_t index)       { return *_world.bodies[index]; }
	const Entity& _entity(size_t index) const { return *_world.bodies[index]; }
	const Entity& _const_entity(size_t index) { return *_world.bodies[index]; }

//!!	bool entity_at(model::Math::Vector2f world_pos, size_t* entity_id OUT) const;
//!!	bool entity_at(model::Math::Vector3f world_pos, size_t* entity_id OUT) const;
//!!	bool entity_at_viewpos(View::Vector2f view_pos, size_t* entity_id OUT) const;
	virtual bool entity_at_viewpos(float x, float y, size_t* entity_id OUT) const;
	virtual bool is_entity_at_viewpos(size_t entity_id, float x, float y) const;

	virtual size_t add_entity(Entity&& temp);     // Move from temporary/template obj.
	virtual size_t add_entity(const Entity& src); // Copy from obj.
	virtual void remove_entity(size_t ndx);

/*!!
	using EntityTransform = void(*)(Entity&);
	using EntityTransform_ByIndex = void(*)(size_t ndx);
	virtual void transform_entity(EntityTransform f) {}
	virtual void transform_entity(EntityTransform_ByIndex f) {}
!!*/

//!!	unsigned add_player(Player&& tempp); // Calls a virtual hook to let the app finish it...
	virtual unsigned add_player(
		Entity&& model,
		Avatar& avatar,
		VirtualController& controls
	) = 0; //!! Ugh... Refine! (Can't really be done nicely in C++, though.)
	virtual void   remove_player(unsigned player_id) = 0; //!this should then be virtual, too (like destructors)
	Player& player(unsigned player_id = 1) {
		assert(player_id > 0); // 1-based player IDs
		assert(players.size());
		assert(players.size() >= player_id); //! >=, not >, for player_id is 1-based!
		return players[player_id - 1];
	}
	const Player& player(unsigned player_id = 1) const { return ((SimApp*)this)->player(player_id); }

	size_t player_entity_ndx(unsigned player_id = 1) const {
		auto ndx = player(player_id).entity_ndx;
		assert(entity_count());
		assert(ndx < entity_count());
		return ndx;
	}
	       Entity& player_entity(unsigned p = 1)       { assert(entity_count() > player_entity_ndx(p)); return entity(player_entity_ndx(p)); }
	 const Entity& player_entity(unsigned p = 1) const { assert(entity_count() > player_entity_ndx(p)); return entity(player_entity_ndx(p)); }

	float player_idle_time(  unsigned player_id = 1) const; // No input for so many seconds (0: busy; gated by cfg.player_idle_threshold)
	bool  player_idle(       unsigned player_id = 1) const { return player_idle_time(player_id) > 0; }
	void  player_mark_active(unsigned player_id = 1);

	//----------------------------------------------------------------------------
	// Model event hooks (callbacks)

	virtual void init_world_hook() {} // Called by world.init().
	/*
	virtual bool collide_hook(World* w, Entity* obj1, Entity* obj2)
	{w, obj1, obj2;
		return false;
	}
	*/
	virtual bool collide_hook(Model::World* w, Entity* obj1, Entity* obj2, double distance);
	virtual bool touch_hook(Model::World* w, Entity* obj1, Entity* obj2);

	// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
	// `event` represents the physical property/condition that made it think these might interact.
	//!!NOTE: This will change to the objects themselves being notified (not the game "superclass")!
	virtual void undirected_interaction_hook(Model::World* w, Entity* obj1, Entity* obj2, float dt, double distance, ...);
	virtual void directed_interaction_hook(Model::World* w, Entity* source, Entity* target, float dt, double distance, ...);


	//----------------------------------------------------------------------------
	//Misc convenience helpers
	auto&       main_window()           { return backend.hci.main_window(); }
	const auto& main_window()     const { return backend.hci.main_window(); }
	unsigned main_window_width()  const { return main_window().cfg.width; }
	unsigned main_window_height() const { return main_window().cfg.height; }

//------------------------------------------------------------------------
// Internals...
//------------------------------------------------------------------------
protected:
	//!! Migrate the Engine-ish impl. parts here and remove the `abstractness`:
	virtual void event_loop() = 0;
	virtual void update_thread_main_loop() = 0;
	virtual void updates_for_next_frame() = 0;
	virtual void onResize(unsigned /*width*/, unsigned /*height*/) {}
	//--------------------
	// Rendering... (See also main_view()!)
	virtual void draw() = 0;
	//!! Render sync. kludge (used e.g. by the general-purpose particle emitter) -> #516...):
		public: virtual void resize_shape(size_t /*ndx*/, float /*factor*/) {}
		public: virtual void resize_shapes(float /*factor*/) {}

//----------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	SimApp(int argc, char** argv, View::ScreenView& main_view);
	virtual ~SimApp();

	SimApp(const SimApp&) = delete;

//----------------------------------------------------------------------------
// Data...
//----------------------------------------------------------------------------
protected:
	Args args;

public: // E.g. the renderer still needs these...
	SimAppConfig cfg;
	Backend& backend;

	//--------------------------------------------------------------------
	// Engine-specific UI that the client app is also free to use
	// The sim/app "content" has its own rendering, most likely its own UI too,
	// but usually sharing the same (currently: SFML) window!
protected:
	sfw::GUI gui; //!! Forward-declare only, and the backend-specific impl. ctor should create it... somehow... :)
	              //!! -- e.g. via a unique_ptr, or just a plain manual PIMPL. (Plus a gui() accessor then?!)

	SessionManager session;

	//--------------------------------------------------------------------
	// Abstract (Generic) Model World & View state...

private: // <- Forcing the use of accessors
	Model::World _world; // See the *world() accessors!

	View::ScreenView& _main_view;

protected:
	//--------------------------------------------------------------------
	// Time control...

	Time::Control time;
	sz::CappedCounter<Szim::Time::CycleCount> iterations; // number of model update cycles (from the start of the main (run) loop, or load; !!TBD)
		// 1 calendar year = 3,784,320,000 cycles at 120 fps, so even 32 bits are quite enough!
		// But for longer runs (on persistent-world servers), or for higher resolutions, let's go 64...
	sz::Counter<int> timestepping; // # of scheduled steps to advance/retrace, while in halt (paused) mode
	//!!??Move to the Metrics system and resuse it from there:
	sz::SmoothRollingAverage<0.991f, 1/30.f> avg_frame_delay;
//	sz::RollingAverage<30> avg_frame_delay;

	//--------------------------------------------------------------------
	// Workflow control...

	bool _terminated = false;
	int  _exit_code = 0;
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

	//--------------------------------------------------------------------
	// Player support...
public: //!! <- For the View classes :-/
	std::vector<Avatar> avatars;
	std::vector<Player> players; //!!UNUSED YET, JUST FOR COMPILATION TESTING!!

}; // class SimApp
} // namespace Szim
#endif // _LKLWSJHEWIOHFSDIUGWGHWRTW2245_
