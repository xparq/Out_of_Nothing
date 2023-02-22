#ifndef __SIMAPP__
#define __SIMAPP__

#include "Model/World.hpp"
#include "misc/rolling_average.hpp"
import Storage;

#include <atomic>
#include <format> // vformat

//============================================================================
class SimApp // "Controller"
{
//----------------------------------------------------------------------------
// Config (static)...
//----------------------------------------------------------------------------
protected:
	//!!Move the rest of these to the Model, too, for now:
	//!!static constexpr float CFG_GLOBE_RADIUS = 50000000.0f; // m
	//!!(They will become props initialized from a real config!)
	static constexpr float CFG_THRUST_FORCE = 6e34f; // N (kg*m/s^2)
	static constexpr float CFG_DEFAULT_SCALE = 0.0000005f; //! This one also depends very much on the physics!
	//! See also: World physics! The specific values here depend on the laws there,
	//! so replacing the physics may very well invalidate these! :-o
	//! The depencendies should be formalized e.g. via using virtual units
	//! provided by the physics there!

//----------------------------------------------------------------------------
// Base API...
//----------------------------------------------------------------------------
public:
	virtual bool run() = 0;

	virtual bool poll_and_process_controls() { return false; }

	auto toggle_physics()  { _paused = !_paused; pause_physics(_paused); }
	auto physics_paused()  { return _paused; }
	virtual void pause_physics(bool state = true) { _paused = state; }; //! override to stop the actual world...

	virtual size_t add_player(Model::World::Body&&) = 0; //!!Questionable "generic config" input type!... ;)
	                //!! Bbut C++ doesn't have the covariance needed here.
	                //!! (Still added this cringy fn. for consistency.)
	virtual void   remove_player(size_t ndx) = 0; //!this should then be virtual, too (like destructors)

	auto terminate()  { _terminated = true; }
	auto terminated()  { return _terminated; }

	Model::World const& get_world() const;
	Model::World& get_world();
	void set_world(Model::World const&);
	void set_world(Model::World &);

	virtual bool save_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	virtual bool load_snapshot(unsigned slot = 1); // 1 <= slot <= MAX_WORLD_SNAPSHOTS
	//virtual bool save_snapshot(const char* filename);
	//virtual bool load_snapshot(const char* filename);
	template <typename... X>
	std::string snapshot_filename(size_t slot_ndx = 1, const std::string& format = "snapshot_{}.sav", const X... args) {
		return std::vformat(format, std::make_format_args(slot_ndx, args...));
	}

	//----------------------------------------------------------------------------
	// Model event hooks (callbacks)
	/*
	virtual bool collide_hook(World* w, World::Body* obj1, World::Body* obj2)
	{w, obj1, obj2;
		return false;
	}
	*/
	virtual bool collide_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2, float distance);
	virtual bool touch_hook(Model::World* w, Model::World::Body* obj1, Model::World::Body* obj2);

	// High-level, abstract (not as in "generic", but "app-level") hook for n-body interactions:
	// `event` represents the physical property/condition that made it think these might interact.
	//!!NOTE: This will change to the objects themselves being notified (not the game "superclass")!
	virtual void interaction_hook(Model::World* w, Model::World::Event event, Model::World::Body* obj1, Model::World::Body* obj2, ...);

//------------------------------------------------------------------------
//!!Migrate to the UI, as sg. like "controllers/button_state", to uniformly
//!!support more than just a keyboard (mouse, touch, joys, game controllers etc.).
//!!Virtual tap buttons should also be possible to route thru the successor of this ifc.
//!!And then add support for non-latin keyboards, too!...
//!!
//!!Also: provide conversion key code conversion tables for various supported backends (like SFML, SDL, Win32 etc.)
//!!(The same mechanism could also be extended even further to allow layout remapping independently of what's
//!!supported by those.)
public:
	enum KBD_STATE {
		//! -> eg. https://developer.mozilla.org/en-US/docs/Web/API/UI_Events/Keyboard_event_key_values
		//!    or: http://www.kbdedit.com/manual/low_level_vk_list.html

		//! Adding typically non-existent "keys" too, both to follow ASCII,
		//! and as placeholders for layouts that actually do have them.
		//! (And this is our own virtual keyboard anyway, we can do with it whatever we want. ;-p )

		NUL = 0,
		PAUSE = 1,
		
		//!!CONTEXT[_MENU],
		//!!Keypad...
		//!!Media keys...
		//!!Accented keys, obscure labels on "extended" keyboards

		HOME = 2, END,
		UP = 4, DOWN, LEFT, RIGHT,

		BACKSPACE = 8,
		TAB = 9,
		INSERT = 10,
		PAGE_UP = 11, PAGE_DOWN,
		ENTER = 13,

		LSHIFT = 15, RSHIFT, // SHIFT is a meta state, see below
		LCTRL = 18, RCTRL, // CTRL is a meta state, see below
		LALT = 21, RALT, ALTGR = RALT, // ALT is a meta state, see below
		WINDOWS,

		CAPS_LOCK, NUM_LOCK, SCROLL_LOCK, //!!Emulated in OON_sfml now -- move to a non-OON, but SFML-specific event preprocessor!
		                                  //! NOTE: CAPS, NUM & SCROLL give key.code -1 in SFML 2.6.x :-/
		ESCAPE = 27,

		SPACE = 32,

		EXCLAMATION,
		QUOTE,
		NUMBER,
		DOLLAR,
		PERCENT,
		AMPERSAND,
		APOSTROPHE,
		PAREN_LEFT,
		PAREN_RIGHT,
		ASTERISK,
		PLUS,

		COMMA,
		HYPHEN,
		DOT,
		SLASH,					 
		DIGIT0 = 48, DIGIT1, DIGIT2, DIGIT3, DIGIT4, DIGIT5, DIGIT6, DIGIT7, DIGIT8, DIGIT9,
		COLON, SEMICOLON, LESS_THAN, EQUALS, GREATER_THAN, QUESTION_MARK, AT,
		A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		BRACKET_LEFT, BACKSLASH, BRACKET_RIGHT, CARET, UNDERSCORE, BACKQUOTE,
		a = 97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		BRACE_LEFT, PIPE, BRACE_RIGHT, TILDE,
		DELETE = 127, 
		F1 = 129, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16,

		// Meta states:
		SHIFT = 160, CTRL, ALT,
		CAPS_LOCKED, NUM_LOCKED, SCROLL_LOCKED,

		__LAST__ = 254,
		__SIZE__ //! Should be good for a good while...
	};

//------------------------------------------------------------------------
// C++ mechanics...
//----------------------------------------------------------------------------
public:
	SimApp() = default;
	SimApp(const SimApp&) = delete;
	virtual ~SimApp() = default;

//------------------------------------------------------------------------
// Data / Game (World) State...
//----------------------------------------------------------------------------
protected:
	Model::World world;

//------------------------------------------------------------------------
// Data / Internals...
//----------------------------------------------------------------------------
protected:
	bool _terminated = false;
	bool _paused = false;
	//!!Migrate to the Metrics system:
	misc::RollingAverage<50> avg_frame_delay;

	// Player-controls (transient state)
	enum UIEventState { IDLE, BUSY, EVENT_READY };
	std::atomic<UIEventState> ui_event_state{ UIEventState::BUSY }; // https://stackoverflow.com/a/23063862/1479945

	bool kbd_state[KBD_STATE::__LAST__] = {0}; // Can't just be bool, 'coz the doubled modifiers need 2 bits!
};

#endif // __SIMAPP__
