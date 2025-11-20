//
// The Engine API exposed to the app
//

#pragma once

class Args;
//#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?

namespace Szim {
	class RuntimeContext;
	class SimAppConfig;
	class Backend;
}
#include "Szim/UI-fw.hpp"

/*
#include "Szim/App/Config.hpp"
#include "Szim/Backend.hpp"
#include "Szim/Backend/adapter/SFML/_Backend.hpp" //!!... :-/ Use proper dispatching!
#include "Szim/SessionManager.hpp"

#include "Szim/UI-fw.hpp"
//!!#include "Szim/UI/HUD.hpp"
#include "Szim/UI/Input.hpp"
*/

namespace Szim {

using EngineConfig = SimAppConfig; //!! But it's still a mix of both app and engine stuff! :-/

static constexpr auto DEFAULT_CFG_FILE = "default.cfg";

class RuntimeContext
{
public:
	Args&         args;
	EngineConfig& syscfg;
	Backend&      backend;
	//--------------------------------------------------------------------
	// Engine-specific UI that the client app is also free to use
	// The sim/app "content" has its own rendering, most likely its own UI too,
	// but usually sharing the same (currently: SFML) window!
	myco::GUI&     gui;

//	RuntimeContext(int argc, char** argv);
//	RuntimeContext(const RuntimeContext&) = delete;

	template<typename T = RuntimeContext>
	T& as() { return static_cast<T&>(*this); }

}; // RuntimeContext

} // namespace Szim
