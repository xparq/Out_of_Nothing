#ifndef _FDDOUEYGTDGDY2GOUSDTUOHYUIFGUHRWFTFRYEGR_
#define _FDDOUEYGTDGDY2GOUSDTUOHYUIFGUHRWFTFRYEGR_


#include <cstdio> // size_t, too...
#include <assert.h> // <cassert> is incompatible with MSVC's `import std` solution.

namespace sz
{
class File
{
	std::FILE* f_ = 0;
	bool autoclose = true;
	// Reuse existing handle (intended for stdin/stdout/stderr, mostly!):
protected:
	explicit File(std::FILE* h) : f_(h), autoclose(false)  {}
public:
	explicit File(const char* name, const char* flags = "rb") { f_ = std::fopen(name, flags); }
	~File() { if (autoclose && f_) std::fclose(f_); }
	File(const File&) = delete;
	operator bool () const { return f_ && !std::ferror(f_) && eof(); }
	bool eof() const { return f_ ? std::feof(f_) : false; }
	virtual std::size_t write(const char* buf, long long size) { //! std::streamsize count (~ssize_t)
		return handle() ? std::fwrite(buf, size, 1, handle()) : 0;
	}
	void flush() { if (f_) std::fflush(f_); } // fflush(0) would flush every open file! :)
	std::FILE* handle() const {
#ifdef assert
		assert(f_);
#endif
		return f_; }
};

} // namespace sz


#endif _FDDOUEYGTDGDY2GOUSDTUOHYUIFGUHRWFTFRYEGR_
