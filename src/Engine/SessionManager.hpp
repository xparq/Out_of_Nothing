#ifndef _SKDJ03946H1D81DB67ZBV6435_
#define _SKDJ03946H1D81DB67ZBV6435_

#include <string>

namespace Szim {

class SessionManager;
//----------------------------------------------------------------------------
class Session //!!?? Could perhaps even be an inner class of SessionManager
{
friend class SessionManager;
	std::string filename;         // Load(ed) from this, and also save to this by default
	std::string save_as_filename; // Save to this instead, if not empty
	bool autosave = true;         // Save this session on close (otherwise S. Mgr. should ask)

public:
	bool save();
	bool load();

	Session(/*!!const string& path = ""!!*/); // create()
	Session(const Session&) = delete;
};

class SimApp;
//----------------------------------------------------------------------------
class SessionManager
//!!
//!! Should mature to take over some of the raw 'run' logic, so that this
//!! could call the app's init/done virtuals, instead of SimApp directly!
//!! That should allow seamlessly creating/cleaning up (even multiple) app
//!! instances by this class dynamically, as/when needed, along with more
//!! flexibility features (that would be too much for the already fattening
//!! SimApp).
//!!
//!! Support multiple live sessions, switching between them to select the active one! -> #274
//!!
{
public:
	SessionManager(SimApp& app/*!!, const std::string& name!!*/);
	~SessionManager(); // (Just for diagnostics yet)

	SessionManager(const SessionManager&) = delete;

	void create(const std::string& name);      // Start new (unnamed, if name == "")
	void open(const std::string& name);        // Start existing or new (-> create unnamed, if name == "")
	void save();                               // Save (ask for or generate name, if unnamed)
	void save_as(const std::string& name);     // Save (ask for or generate valid name, if name == "")
	void close();                              // Close, possibly saving

	//!! Short of a general-purpose property manager...:
	void set_autosave(bool state);
	void set_save_as_filename(const std::string& fn);

protected:
	SimApp& app; //!! Generic (SimApp-level) "sysapp" (i.e. "process", rather,
	             //!! as no access to "real" client app stuff from here (yet?...
		     //!! as derived SessionManagers may have it; similarly specialized
		     //!! like SimAppConfig, but also with a ref to the client app)
		     //!! Not sure it's a good idea to go that way at all though...
		     //!! Probably much better to call overridden virtuals for things
		     //!! like "build new default app state" for a new session etc.

	std::string active_session_name; //!! Should be a key to the active session in the session list!
	Session active_session;          //!! Should only be a ref to the active session in that list!
};

} // namespace Szim

#endif // _SKDJ03946H1D81DB67ZBV6435_
