#ifndef _UNILANG_HH_
#define _UNILANG_HH_
//
// General language extensions (synonyms, synt. sugar, utilities etc.) for
// ground-levelling across projects, platforms, impl. languages etc.
//
//!! (Amazingly, after ~30 years in SW engineering, I find myself writing
//!! this memo to myself, as a reminder, or focusing aid... It's like a
//!! craftsman recognizing the importance of a toolbox... I have no words.)
//!!
//!! This should grow into my own personal "stdlib" that can be used by
//!! default (i.e.: without thinking twice) in any new C++ project of mine.
//!! (Model: Jan Haller's Aurora; among infinite others...)
//!!
//!! Also: the "language" (concepts, IDs, features) defined here should be
//!! independent of C++ by nature, even tnough in this particular version
//!! (i.e. "unilang for C++") they obviously aren't. But it should still be
//!! considered in the context of the bigger picture, as a unified abstract
//!! language, independent of any particular *approximation* of it.
//!!
//!! OTOH, for any particular impl. language, specific extensions (that make
//!! sense only there) should also be added, if they are generic enough (in
//!! the context of that lang.) to make life easier across any projects.
//!!
//!! (IOW, this is what others often call the "foundation" or even (slightly
//!! confusingly) "platform" layer.)
//!!
//!! -> https://github.com/x1ab/cpp/blob/main/WISHLIST.txt
//

//!!#include "a proper enum..."

//!! Preprocessor:
//!! Add preprocessor warnings/errors for MSVC legacy PP without __VA_OPT__ etc...
//!! Shim for missing #warning...
//!! ...

#define AUTOCONST constexpr static auto

namespace sz // `using namespace Sz;` is encouraged... The client code's also mine, after all! ;)
{

//
// `fallthrough` control-flow statement (macroless version):
//
constexpr static class{} fallthrough [[maybe_unused]];

// On/Off synonyms for true/false
//!!ProperEnum { Off, On };
AUTOCONST Off = false, On = true; // Not the crippled C++ stock enum; autoconvert needed!
//!!??enum { UseDefault = -1 };

} // namespace
#endif //_UNILANG_HH_
