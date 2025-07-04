#pragma once

// The App interface (SimApp base):
//#include "Engine/SimApp.hpp"
namespace Szim { class SimApp; };

// The Engine API exposed to the app:
#include "Engine/RuntimeContext.hpp"

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"

#include <utility>
#include <cassert>


namespace Szim {

	constexpr inline int SmileyFace = 0;
	constexpr inline int SadFace = -1;

class Engine : public RuntimeContext
{
public: //!!?? Maybe I can actually get away with the convention of no pre-/postfix underscore in *public* member names (just like in classic structs)...?

	SimApp* app_ = nullptr; // Not App* to a) enforce using the SimApp interface, and b) minimize knowledge about App even within this class!
	                        // The current app; while this is not null, another run() call is not allowed.

	bool    app_implicitly_created_ = true;


	//--------------------------------------------------------------------
	// Apart from the run() API, this is (currently) the only place that need to know about the real App type,
	// in order to construct it.
	template<class App, typename... Args>
	void __create_app_implicitly(Args&&... app_init_args) //!! Sorry about yet another level of templated forwarding cringe!...
	{
		assert(!app_);

		app_ = new App(*this, std::forward<Args>(app_init_args)...);

		if (app_) {
			app_implicitly_created_ = true;
		} else {
			app_implicitly_created_ = false;
			assert(app_); //! What else, anyway?... Any non-debug reporting is up to the caller.
		}
	}

	//--------------------------------------------------------------------
	void __delete_app_if_implicit();

	//--------------------------------------------------------------------
	int __run();
	// Note: the generalized param. forwarding here (for passing them to init), is NOT
	// to avoid knowing about the App type (which we don't here), but simply to decouple
	// from any internal changes possibly altering the polymorphic SimApp::init(...) signature!
	//--------------------------------------------------------------------

public:
	//--------------------------------------------------------------------
	Engine(int argc, char** argv)
	// Note: not creating an app instance here, because the engine can have (run) more than 1 app during its lifetime actually.
	// Well, at least conceptually.
		: RuntimeContext(argc, argv)
	{
		Note("SimEngine initialized.");

		//! Note: no compulsory __create_app here; we're doing lazy 2-stage init!
	}

	//--------------------------------------------------------------------
	~Engine()
	{
		//!! Don't let ecxeptions leave the dtor, that'd be a gamble against a double-fault termination.
		try {
			shutdown();
			//!! See notes at startup() in run()!
			//!! Putting shutdown here despite startup() is not in the ctor, for the same reason
			//!! the create/delete logic is also asymmetrical! (To support deferred/lazy init.)
		} catch(...) {
			Bug("Unhandled exception during Engine Shutdown!");
		}
	}


	//--------------------------------------------------------------------
	void startup();
	void shutdown();

	//--------------------------------------------------------------------
	// Take an existing App, then init & run it...
	template<class App, typename... Args>
	int run(App& app, Args&&... app_init_args)
	{
		//!!return __run(app, std::forward<Args>(app_init_args)...);
		//!! Ahh, but it can't be templated, as __run needs to go into a separate TU!... :-/
		//!! Fortunately, if we take an existing App, we can expect it to have been initialized already
		//!! with its custom-args ctor, so __run() can just go aghead and call its uniform init()!

		return __run();
	}

	//--------------------------------------------------------------------
	// Create a new App, and run it...
	template<class App, typename... Args>
	int run(Args&&... app_init_args)
	{
		if (app_) { // Did I mention repeated app runs?... ;)
		            // Note: not just assert(), as this is a serious moment in all of our lives,
		            // and we have all the freedom here to stop and ponder also in NDEBUG.
			Bug("Attempting to create an app, while one already exists!");
			return SadFace;
		}

		__create_app_implicitly<App>(std::forward<Args>(app_init_args)...);
		if (!app_) {
			Error("What? Couldn't create App?!");
			return SadFace;
		}

		int exit_code = __run();

		__delete_app_if_implicit(); //! This create/delete ritual may later evolve out from run,
		                            //! in case you were wondering about the overengineering. ;) (Yes, I've considered YAGNI.)

		return exit_code;
	}

#if 0
	//--------------------------------------------------------------------
	template<typename... Args>
	int __init(/*!!?? SimApp& app, ??!!*/Args&&... init_args)
	// Note: the generalized param. forwarding here (for passing them to init), is NOT
	// to avoid knowing about the App type (which we don't here), but simply to decouple
	// from any internal changes possibly altering the polymorphic SimApp::init(...) signature!
	//--------------------------------------------------------------------
	{
		if (! &app_) {
			Bug("Nice try running nullptr as 'App'!...");
			return SadFace;
		}
//!!NEW:	int exit_code = app_->init(std::forward<Args>(app_init_args)...);
//!!OLD:
		int exit_code = SadFace;
		app_->init();
		exit_code = app_->exitcode(); //!! init() should just return that...
		if (!app_->terminated() && exit_code == SmileyFace) { //!!?? Or... exit_code may be "dirty" without wanting to exit in the old SimApp design?
			Note("Application initialized.");
		} else {
			Error("Application setup failed.");
		}

		return exit_code;
	}
#endif
}; // class Engine

} // namespace Szim
