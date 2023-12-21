#ifndef _SOIUEYTB6729STVWO8E7RBYG968WRY6498754_
#define _SOIUEYTB6729STVWO8E7RBYG968WRY6498754_

#include <cstdint>

namespace Szim {

class Avatar
{
public:
	const char* image_path = "";
	uint32_t    tint_RGBA = 0xffffffff; // (Alpha affects texture transparency, it's not a weight for the bg. color!)
	const char* entry_music_path = "";

//!!	virtual bool load(const char* prefix_path = "") = 0; // Can't have virtuals for the brittle C++ desig. init support! :-/
};

} // namespace Szim

#endif // _SOIUEYTB6729STVWO8E7RBYG968WRY6498754_