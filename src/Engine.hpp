#pragma once

// The App interface (SimApp base):
namespace Szim { class SimApp; };
//#include "Engine/App.hpp"

// The Engine API exposed to the app
#include "Engine/RuntimeContext.hpp"

// Special-casing these to not be u.ptr like the others (it was too much annoyance for too little):
#include "extern/Args.hpp" //!!?? move to sz:: or absorb directly by Szim?

#include <memory> // unique_ptr
//! For the templated implementations:
#include "Engine/diag/Error.hpp"
#include <utility> // forward
#include <cassert>


namespace Szim {

	constexpr inline int SmileyFace = 0;
	constexpr inline int SadFace = -1;

class Engine //!!: public RuntimeContext
{
	bool    __engine_initialized_ = false; // Flipped to true in startup(), just for debugging/asserting...

public: //!!?? Maybe I can actually get away with the convention of no pre-/postfix underscore in *public* member names (just like in classic structs)...?

	SimApp* app_ = nullptr; // Not App* to a) enforce using the SimApp interface, and b) minimize knowledge about App even within this class!
	                        // The current app; while this is not null, another run() call is not allowed.

	bool    app_implicitly_created_ = true;


	// Services impl. Move these from this generic class to a shed in the shade!
	//!!std::unique_ptr<Args> args;
	//!! Just too much hassle to make this also a uniq.ptr:
	Args args;
	std::unique_ptr<EngineConfig> syscfg;
	//!!std::unique_ptr<Backend> backend;
	//!! Just too much hassle to make this also a uniq.ptr:
	Backend* backend; // Static singleton from a factory (::use(...))...
	                  //!! But can't be a ref, as it will be initialized later in startup(), not the ctor!
	std::unique_ptr<myco::GUI> gui;

/*!! This is probably just a dream, without eventually needing to #include SimApp here...:
	//--------------------------------------------------------------------
	// Hybrid Comp.-time/Runtime polymorphic interface to App's init(...)
	//
	// - If init(...) matches <Args>, it is called.
	// - If it doesn't, init() is called.
	//   Since App is derived from SimApp, it's always available.
	//   (And, of course, it can also be freely overridden.)
	//
	// Note: App is passed as a param, for ADT, rather than using the
	//       internal app_ pointer, which is type-erased to SimApp.
	//
	template<typename App, typename... Args> requires
		requires(App& app, Args&&... args) {
			{ app.init(std::forward<Args>(args)...) };
		}
	auto __call_app_init(App& app, Args&&... args)\
	{
		return app.init(std::forward<Args>(args)...);
	}
	// Fallback to the classic virtual override in the app base:
	template<typename App, typename... Args> requires
		(!requires(App& app, Args&&... args) {
			{ app.init(std::forward<Args>(args)...) };
		})
	auto __call_app_init(App& app, Args&&...)
	{
		return app.init();  // possibly virtual
	}
!!*/
	//--------------------------------------------------------------------
	// Apart from the run() API, this is (currently) the only place that need to know about the real App type,
	// in order to construct it.
	template<class App, typename... Args>
	void __create_app_implicitly(Args&&... app_init_args)
	{
		assert(__engine_initialized_);
		assert(!app_);

		app_ = new App(
			RuntimeContext{
				args, // regular member
				*syscfg, // unique_ptr
				*backend, // plain ptr
				*gui  // unique_ptr
			},
			std::forward<Args>(app_init_args)...);

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
	Engine(int argc, char** argv); // Just store the args for the actual init in startup()...
		// Not creating an app instance here, because the engine can have (run) more than 1 during its lifetime.
		// (Well, at least that's the plan.) Also, we're doing a 2-stage lazy init anyway.
	~Engine();

	//--------------------------------------------------------------------
	void startup();
	void shutdown();

	//--------------------------------------------------------------------
	// Take an existing App, then init & run it...
	template<class App, typename... Args>
	int run(App& app, Args&&... app_init_args)
	{
		startup(); // We may have just arrived to the engine for the first time!

		//!!return __run(app, std::forward<Args>(app_init_args)...);
		//!! Ahh, but it can't be templated, as __run needs to go into a separate TU!... :-/
		//!! Fortunately, if we take an existing App, we can expect it to have been initialized already
		//!! with its custom-args ctor, so __run() can just go ahead and call its uniform init()!

		//!! __call_app_init(app, std::forward<Args>(app_init_args))

		return __run();
	}

	//--------------------------------------------------------------------
	// Create a new App, and run it...
	template<class App, typename... Args>
	int run(Args&&... app_init_args)
	{
		startup(); // We may have just arrived to the engine for the first time!

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
