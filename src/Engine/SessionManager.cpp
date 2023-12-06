#include "SessionManager.hpp"
#include "SimApp.hpp"
#include "sz/fs.hh"

#include <string>
#include <iostream>
#include <cassert>

using namespace Szim;
using namespace std;


//============================================================================
//----------------------------------------------------------------------------
Session::Session(/*!!const string& path = ""!!*/)
{
}


//============================================================================
//----------------------------------------------------------------------------
SessionManager::SessionManager(SimApp& app) :
	app(app)
//	active_session{.filename = ...} // Leave the active session empty for now!
{
cerr << "+++ SessionManager started.\n";
}

//----------------------------------------------------------------------------
SessionManager::~SessionManager()
{
cerr << "+++ SessionManager stopped.\n";
}


//----------------------------------------------------------------------------
void SessionManager::set_autosave(bool state)
{
	active_session.autosave = state;
}

void SessionManager::set_save_as_filename(const string& fn)
{
	active_session.save_as_filename =
		sz::prefix_if_rel(app.cfg.session_dir, fn);
		//!! This manual dir prefixing will need to be normalized...
		//!! See notes in open()!
}


//----------------------------------------------------------------------------
void SessionManager::create(const string& /*!!session_name!!*/)
{
cerr << __FUNCTION__ << ": New sessions are not yet \"created\"! They just happen!... :) \n";
}

//----------------------------------------------------------------------------
void SessionManager::open(const string& session_name/* = ""*/)
{
cerr << __FUNCTION__; // To be continued...

	//!! Should handle things like:
	//!! - saving an existing session first
	//!! - starting new named session if 'name' has no file yet (-> create(""))
	//!! - ...

	active_session_name = session_name;

	if (active_session_name.empty()) {
		create("");
cerr << " starting new session\n";
		return;
	}

	//!! Use the same prefixing logic as the Config etc.
	//!! -- JUST NOT THIS WAY, BUT VIA A SYSTEM-LEVEL RES. MGR.!
	active_session.filename =
		sz::prefix_if_rel(app.cfg.session_dir, active_session_name);
		//!! This manual dir prefixing will need to be normalized,
		//!! i.e. synced with snapshot_filename(), by both that and
		//!! this calling the same unified asset/resource filename
		//!! resolver function... -> #257

cerr << " " << active_session.filename << '\n';

	if (!app.load_snapshot(active_session.filename.c_str())) {
		cerr << __FUNCTION__ << ": Failed to load session state (from "<<active_session.filename<<")!\n";
		//!!??... create(name)
	}
}

//----------------------------------------------------------------------------
void SessionManager::close()
{
cerr << __FUNCTION__; // To be continued...

	if (active_session.autosave) {

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

cerr << " with autosave to \"" << save_as << "\"\n";

		if (!app.save_snapshot(save_as.c_str())) {
			cerr << __FUNCTION__ << ": Failed to save session state (to "<<save_as<<")!\n";
		}
	} else {
cerr << '\n';
	}
}
