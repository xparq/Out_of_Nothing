#include "SessionManager.hpp"
#include "Szim/SimApp.hpp"

#include "Szim/diag/Error.hpp"
#include "Szim/diag/Log.hpp"
#include "sz/diag/DBG.hh"

#include "sz/sys/fs.hh"

#include <string>
#include <cassert>


namespace Szim {
namespace Core {

using namespace std;


//============================================================================
//----------------------------------------------------------------------------
SessionState::SessionState(/*!!const string& path = ""!!*/)
{
}


//============================================================================
//----------------------------------------------------------------------------
SessionManager::SessionManager(SimApp& app)
	: app_(app)
//	, active_session{.filename = ...} // Leave the active session empty for now!
{
	LOGI << "+++ SessionManager started.";
}

//----------------------------------------------------------------------------
SessionManager::~SessionManager()
{
	LOGI << "+++ SessionManager stopped.";
}


//----------------------------------------------------------------------------
void SessionManager::set_autosave(bool state)
{
	active_session.autosave = state;
}

void SessionManager::set_save_as_filename(const string& fn)
{
/*!! DISABLED FOR #555 (Double-prefixed session paths...)
     save_snapshot() will do the prefixing! (Which likely needs to change later!)

	active_session.save_as_filename =
		sz::fs::prefix_by_intent(app.cfg.session_dir, fn);
		//!! This manual dir prefixing will need to be normalized...
		//!! See notes in open()!
!!*/
	active_session.save_as_filename = fn;
}


//----------------------------------------------------------------------------
void SessionManager::create(const string& /*!!session_name!!*/)
{
LOGD << "New sessions are not yet \"created\"! They just happen!... :)";
}

//----------------------------------------------------------------------------
bool SessionManager::open_preps(const string& session_name/* = ""*/)
{
	LOGD << "Session Open...";

	//!! Should handle things like:
	//!! - saving an existing session first
	//!! - starting new named session if 'name' has no file yet (-> create(""))
	//!! - ...

	active_session_name = session_name;

	if (active_session_name.empty()) {
		create("");
		LOGD << "...as new session!\n";
		return false;
	}
/*!! DISABLED FOR #555 (Double-prefixed session paths...)
     load_snapshot() will do the prefixing! (Which likely needs to change later!)
	//!! Use the same prefixing logic as the Config etc.
	//!! -- JUST NOT THIS WAY, BUT VIA A SYSTEM-LEVEL RES. MGR.!
	active_session.filename =
		sz::fs::prefix_by_intent(app.cfg.session_dir, active_session_name);
		//!! This manual dir prefixing will need to be normalized,
		//!! i.e. synced with snapshot_filename(), by both that and
		//!! this calling the same unified asset/resource filename
		//!! resolver function... -> #257
!!*/
	active_session.filename = active_session_name; //!! #555 load_snapshot *will* prefix it!

	LOGD << "...from session file: " << active_session.filename << '\n';

	return true;
}

//----------------------------------------------------------------------------
bool SessionManager::close_preps()
{
	LOGD << "Session Close...";

	if (!active_session.autosave) {
		//!! LOG_LINE_END //cerr << '\n';
		return false;
	} else {

		/*!!?? if (name.empty()) {

			... What now? It's a mess...
			Even tho no name, there's no control about setting any filenames "directly"
			-- which is exactly what's being done currently by SimApp::init()...

			//!! Not create(), as that would reset the session *state*, too, not just the name! :)
			...
		}!!*/

		//!!
		//!! IMPLICIT ASSUMPTION ALERT:
		//!!
		//!! If a filename has been set, it's assumed to have already been prefixed.
		//!! But no control over that anywhere yet.
		//!! But can't even check for that easily, as the prefix itself can be a relative path!
		//!! (So repeated prefixing is not an option either...)
		//!!

		string& save_as = active_session.save_as_filename; // Just a shorthand

		if (save_as.empty()) {
		    save_as = active_session.filename; //!! Nobody actually sets this (yet), but for completeness..
		}
		if (save_as.empty()) { // Still nothing?
			//!! Ask, or conjure up a filename...
			set_save_as_filename("UNNAMED.autosave"); //!! Just a raw world snapshot yet (not a real "session" file)!
		}
		assert(!save_as.empty());

		LOGD << "...with autosave to \"" << save_as << "\"";

		return true;
	}
}

} // namespace Core
} // namespace Szim
