#pragma once

#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?
#include "Engine/SimAppConfig.hpp"

#include "Engine/Backend.hpp"
#include "Engine/Backend/adapter/SFML/_Backend.hpp" //!!... :-/ Use proper dispatching!
#include "Engine/SessionManager.hpp"
#include "sfw/GUI.hpp"//!! REMOVE FROM HERE! (After hiding it behind a ref., those
                      //!! (mostly?) client .cpps that use it should include it individually!)
//!!#include "Engine/UI/HUD.hpp"
#include "Engine/UI/Input.hpp"


namespace Szim {

using EngineConfig = SimAppConfig; //!! But it's still a mix of both app and engine stuff! :-/

static constexpr auto DEFAULT_CFG_FILE = "default.cfg";


struct RuntimeContext
{
	Args args;
	EngineConfig cfg;

//private:
	Backend& backend;

	//--------------------------------------------------------------------
	// Engine-specific UI that the client app is also free to use
	// The sim/app "content" has its own rendering, most likely its own UI too,
	// but usually sharing the same (currently: SFML) window!
//protected:
	sfw::GUI gui; //!! Forward-declare only, and the backend-specific impl. ctor should create it... somehow... :)
	              //!! -- e.g. via a unique_ptr, or just a plain manual PIMPL. (Plus a gui() accessor then?!)

//!!?? Move this here too?
//!!??	SessionManager session;


	RuntimeContext(int argc, char** argv);

	RuntimeContext(const RuntimeContext&) = delete;

	// Let's have a supported way of casting back to Engine, if needed...
	template<typename T = RuntimeContext>
	T& as() { return static_cast<T&>(*this); }

}; // RuntimeContext

} // namespace Szim
