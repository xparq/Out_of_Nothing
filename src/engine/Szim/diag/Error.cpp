#ifndef _MSC_VER
# warning szlib has a broken build now, so the compilation of its error.cc is done here by a proxy/wrapper .cpp!...
#endif

#include "Error.hpp" // Must define the LOG* macros for error.hh first!... :-o
#include "extern/sz/diag/error.cc" //!! NOTE: This will *reinclude* error.hh (after Error.hpp already did so!), redefining its own macros!... :)
