#ifndef _CSKDJ03946H1D81DB67ZBV6435_
#define _CSKDJ03946H1D81DB67ZBV6435_

/*
	See the templated wrapper (Szim/SessionManager.hpp)!
*/

#include <string>

namespace Szim {

class SimApp;

namespace Core {

//----------------------------------------------------------------------------
struct SessionState //!!?? Could perhaps be an inner class of SessionManager
{
	std::string filename;         // Load(ed) from this, and also save to this by default
	std::string save_as_filename; // Save to this instead, if not empty
	bool autosave = true;         // Save this session on close (otherwise S. Mgr. should ask)

	SessionState(/*!!const string& path = ""!!*/); // create()
	SessionState(const SessionState&) = delete;
};

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
//!! #274: Support multiple live sessions, switching between them to select the active one!
//!!
{
public:
	// Also for diagnostics:
	SessionManager(SimApp& app/*!!, const std::string& session_name!!*/);
	virtual ~SessionManager(); // virtual as a precaution...

	SessionManager(const SessionManager&) = delete;

	void create(const std::string& session_name);      // Start new (unnamed, if name == "")

	bool open_preps(const std::string& session_name);  // Start an existing or new (-> create unnamed, if name == "")
	bool close_preps();

	//!! Short of a general-purpose property manager...:
	void set_autosave(bool state);
	void set_save_as_filename(const std::string& fn);

//protected: //!! We need access from SessionWrapper! :-/
	SimApp& app_;

	std::string active_session_name; //!! Should be a key to the active session in the session list!
	SessionState active_session;          //!! Should only be a ref to the active session in that list!
};

} // namespace Core
} // namespace Szim

#endif // _CSKDJ03946H1D81DB67ZBV6435_
