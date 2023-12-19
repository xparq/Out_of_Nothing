#ifndef _WILIRTHG029828Y9VCY37829045YGCM4EDF_
#define _WILIRTHG029828Y9VCY37829045YGCM4EDF_

#include "OON.hpp"

#include "OONMainDisplay_sfml.hpp"

#include "sfw/GUI.hpp"
#include "UI/hud_sfml.hpp"

#include <utility> // std::unreachable

namespace OON {

//============================================================================

namespace _internal {
struct FUCpp_ViewHack {
	struct _oon_view_container {
		OONMainDisplay_sfml _oon_main_view;
		_oon_view_container(OON_sfml& app);
	} _oon_view;
	FUCpp_ViewHack(OON_sfml& app);
};
}

class OON_sfml : private _internal::FUCpp_ViewHack, public OONApp
{
//--------------------------------------------------------------------
// SFML-specific overrides...
//--------------------------------------------------------------------
protected:
	virtual void time_step(int steps) override;
	void pause_hook(bool newstate) override;
	bool load_snapshot(const char* fname) override;
	//! The generic version of this is enough for now:
	//virtual bool save_snapshot(const char* fname) override;

	//!! DeSFMLize these & move to OONApp:
	void event_loop() override;
	void update_thread_main_loop() override;
	void updates_for_next_frame() override;
	void draw() override;

//------------------------------------------------------------------------
// C++ mechanics...
//------------------------------------------------------------------------
public:
	OON_sfml(int argc, char** argv);
	OON_sfml(const OON_sfml&) = delete;

//------------------------------------------------------------------------
// Data / Internals...
//------------------------------------------------------------------------
protected:

#ifndef DISABLE_HUD
//!!	UI::HUD& ...;
	UI::HUD_SFML timing_hud;
	UI::HUD_SFML world_hud;
	UI::HUD_SFML view_hud;
	UI::HUD_SFML object_hud;
	UI::HUD_SFML help_hud;
	UI::HUD_SFML debug_hud;

	UI::HUD& ui_gebi(HUD_ID which) override { switch (which) {
		case TimingStats: return timing_hud;
		case WorldData:   return world_hud;
		case ViewData:    return view_hud;
		case ObjectData:  return object_hud;
		case HelpPanel:   return help_hud;
		case Debug:       return debug_hud;
		default: std::unreachable(); // c++23 only; and this will be c++999: [[unreachable]]
			//return help_hud; // Dummy, to shut up some pre-c++23 compiler warnings
	}}
#endif
};

} // namespace OON

#endif // _WILIRTHG029828Y9VCY37829045YGCM4EDF_
