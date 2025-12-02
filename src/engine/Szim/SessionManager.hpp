#ifndef _SSDFGM986YSKDJ03946H1D81DB67ZBV6435_
#define _SSDFGM986YSKDJ03946H1D81DB67ZBV6435_

/*
	The use of the word "session" is NOT limited to the saved state itself,
	but models the entire run-time context of a... well: session..., possibly
	even shared by multiple simultaneous users, and serverd by multiple hosts.

	Some (especially shared) sessions may not even be possible to save -- only
	record -- meaningfully. (The difference being: saving is for continuing a
	session, while recording only preserves (certain forms of) data history,
	mainly for analysis or entertainment (e.g. playback) purposes.)
*/

#include "Core/SessionManager.hpp"
#include "Szim/diag/Error.hpp"
#include "Szim/diag/Log.hpp"

namespace Szim {

//----------------------------------------------------------------------------
template <class App>
class SessionWrapper
//!!
//!! Should mature to take over some of the raw 'run' logic, so that this
//!! could call the app's init/done virtuals, instead of SimApp directly!
//!! That should allow seamlessly creating/cleaning up (even multiple) app
//!! instances by this class dynamically, as/when needed, along with more
//!! flexibility features (that would be too much for the already fattening
//!! SimApp).
//!!
//!! #274: Support multiple live sessions, switching between them to select the active one!
//!!
{
	Core::SessionManager& session_;

public:
	SessionWrapper(Core::SessionManager& session)
		: session_(session)
	{}

	App& app() { return static_cast<App&>(session_.app_); }

	// Start existing or new (-> create unnamed, if name == "")
	void open(const std::string& session_name/* = ""*/)
	{
		if (!session_.open_preps(session_name)) return;

		if (!app().load_snapshot(session_.active_session.filename.c_str())) {
			Error("Failed to load session (from " + session_.active_session.filename +")!");
			//!!??... create(name)
		}
	}

	//!!void save();                               // Save (ask for or generate name, if unnamed)
	//!!void save_as(const std::string& name);     // Save (ask for or generate valid name, if name == "")

	// Close, possibly saving
	void close()
	{
		if (!session_.close_preps()) return;

		if (!app().save_snapshot(session_.active_session.save_as_filename.c_str())) {
			Error("Failed to save session state (to " + session_.active_session.save_as_filename + ")!");
		}
	}
};

} // namespace Szim

#endif // _SSDFGM986YSKDJ03946H1D81DB67ZBV6435_
