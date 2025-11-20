//
// REMEMBER:
//
//	Logging is for introspection, not for *user-facing* error or status reports!
//	UI-level error/status reporting is a *separate* service/API!
//
//	Similarly (but at the "insider end" of the spectrum): debug tooling for verification
//	(DbC checks, assertions), as most other diagnostic tooling (debug instrumentation,
//	profiling etc.) are mostly *orthogonal* concerns to logging. (E.g. they even have their
//	own control logic: on/off switches, trigger conditions etc.) Those should merely *use*
//	some means of reporting, not implement their own! And, since they're also introspection
//	tools, a general-purpose logging facility is a great fit. The ideal logging solution
//	should be just a flexible IO exchange, concerned only with fast, enriched messaging
//	and filing logistics — completely agnostic to e.g. "debugging", or even concepts
//	like "errors/warnings" etc.
//	(Note: then the task of "debug logging" would just naturally become what it should've
//	always been: an application, a use case of logging: a specific integration/configuration
//	point *for the purposes* of debugging.)
//
//
//!! If a component also uses sz/log *in its headers*, that might cause conflicts!
//!! Especially if it's configured differently there! (Not to mention the potential
//!! (i.e. "likely") macro collisions in general, regardless of using sz/log...)
//!! Logging is an annoyingly global service across an entire complex project, so
//!! mind that, when integrating components with their own logging solutions! Esp.
//!! those _seemigly_ compatible, by also using the same service backends, but (as
//!! hinted above) with different configurations, or other subtle differences with
//!! a high risk of silent inconsistencies!
//!!
//!! Mind also the annoying entanglement between logging vs. DBG tooling/reporting,
//!! logging vs. error handling/reporting, etc.!
//

#define SZ_LOG_KEEP_THE_DEBUG_MACROS_DESPITE_NDEBUG
#define SZ_LOG_USE_DEFAULT_LEVEL info
//#define SZ_NO_LOG_MACROS
//#define SZ_LOG_DISABLE
#define  SZ_LOG_REPLACE_IOSTREAM

#define SZ_LOG_BUILD // Define this for its impl. parts to be precompiled, rather than being header-only!
                     // See more in Log.cop!
#include "extern/sz/diag/log.hh"

// Imports...
namespace Szim::diag { namespace log = sz::log; }
