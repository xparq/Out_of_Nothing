//----------------------------------------------------------------------------
// My old, "meh, good enough" C++ debug helper macros
// 2.3.5 (Public Domain; github.com/xparq/DBG.hh)
//----------------------------------------------------------------------------
//
// CONTROL SYMBOLS:
//
// !> NEVER SET THEM DIRECTLY IN INDIVIDUAL TRANSLATION UNITS, IF THERE ARE <!
// !> MORE THAN ONE, RISKING MISMATCHED BINARIES (CAUSING SILENT CRASHES)!  <!
// !> SET THEM IN A COMMON CFG. HEADER INSTEAD, OR PREFERABLY: RIGHT HERE!  <!
//
//	DBG_OFF, or NDEBUG     : Turn off everything, compile-time
//	DBG_CFG_NO_WIN32       : Disable Win. specific features (and don't load
//	                         WINDOWS.H), despite running on Windows.
//	DBG_TRACE_OFF          : Turn off stack tracing completely
//	DBG_TRACE_SILENT       : Turn off tracing regular scope transitions
//	                         (only print stack dump on terminate())
//	DBG_CFG_DEVICE         : Dbg::Out::DEBUG_CONSOLE, Dbg::Out::POPUP_MESSAGE,
//			         and Dbg::Out::STD_STREAM
//	DBG_CFG_REPLACE_IOSTREAM : ...with my 100-line toy version
//	                       : Alas, std::string etc. still pull it in! :-/
//
//	DBG_CFG_TRACEDEVICE    : (default: DBG_CFG_DEVICE)
//	DBG_CFG_STREAMPREFIX   : Line prefix used when printing to a
//	                         standard stream (default: "DBG> ")
//	DBG_CFG_DBGCONPREFIX   : Line prefix used when printing to dbg. console
//	                         (default: "DBG> ")
//	DBG_CFG_MSGBOXPREFIX   : Line prefix used when printing to MessageBox
//	                         (default: <none>)
//	DBG_CFG_TRACELINE_PREFIX
//	                       : (default: "")
//	DBG_CFG_SCOPEINDENT_CHAR
//	                       : (default:  ' ')
// OPERATIONS:
//
//	DBG x, ...             : Dump a list of things, with added commas,
//	DBG x << ...           : ...or without commas (also for other dumps!)
//	                       : Also: , can follow << (but not vice versa).
//	                       : Note: DBG << x doesn't work (only DBG x), sorry!
//
//	DBGDUMP x, ...         : Location-tagged dump list, with added commas
//	DBGBOX x, ...          : Same for a popup box (Windows-only!)
//
//	DBG_(expr)             : Dump single item as: DBG> expr: value
//
//	DBG_throw(x)           : Throw with added debug output
//	DBGTHROW(x)            : Throw ONLY in a DEBUG build
//	DBGASSERT(x)           : Throws an exception if fails. (See #42 though!)
//	DBGASSERT_IF(cond, x)  : Only assert x if `cond` is true. (Equivalent
//	                         to ASSERT(cond ? x : true).)
//	DBGABORT               : "commented" abort
//	DBGMARK                : Emit An "I was here" trace mark.
//	DBGBEEP                : Just a beep
//
//	DBG_ENABLE/DBG_DISABLE : Turn on/off all output, run-time. (Repeated DISABLE
//	                         calls will need the same number or ENABLE calls!)
//	DBGTRACE               : Push a scope trace marker
//	DBGTRACE_QUIET         : Turn to silent scope tracing
//	DBGTRACE_LOUD          : Turn to verbose scope tracing
//
//	DBGLOG_USE(fname)      : Select file for subsequent DBGLOG writes.
//	                       : Optional. Default: debug.log
//	DBGLOG x, ...          : Log to file, with added commas, or
//	DBGLOG x << ...        : ...without commas.
//	DBGLOG_(expr)          : Log as expr = value
//
//	DBGLOGX(fname) x, ...  : Like DBGLOG, but thread-safe!
//	DBGLOGX_(fname, expr)  : Like DBGLOG_, but thread-safe!
//
// Obsolete/broken:
//	DBGFILE(const char* filename, const char* fmt, ...)
//
// PLATFORM-SPECIFIC:
// - Win32:
//	DBG_CFG_[TRACE]DEVICE == Dbg::Out::DEBUG_CONSOLE --> OutputDebugString
//	DBG_CFG_[TRACE]DEVICE == Dbg::Out::POPUP_MESSAGE --> MessageBox
//	DBGBEEP --> MessageBeep(-1); : (system default beep)
//
//----------------------------------------------------------------------------

#ifndef _DBG_HH_NC249T87DN2376T27E64N5978Y35987_
#define _DBG_HH_NC249T87DN2376T27E64N5978Y35987_


// ==========================================================================
//
//                            * Things common for DEBUG ON/OFF *
//
// ==========================================================================
//struct X_Invariant_Violation { X_Invariant_Violation(const char* expr); };
//struct DBG_X_ASSERT {};


#if defined(NDEBUG) && !defined(DBG_OFF)
#define DBG_OFF
#endif

#if defined(DBG_OFF)
// ==========================================================================
//
//                             * DEBUG OFF *
//
// ==========================================================================

namespace {
	struct _NoDbg_Sink_ { static constexpr void NOOP(...) {} };
	template<typename T> _NoDbg_Sink_ constexpr operator << (_NoDbg_Sink_&&, [[maybe_unused]] T&& x) { return _NoDbg_Sink_(); }
	template<typename T> _NoDbg_Sink_ constexpr operator ,  (_NoDbg_Sink_&&, [[maybe_unused]] T&& x) { return _NoDbg_Sink_(); }
}

#define DBG_ENABLE
#define DBG_DISABLE

#define DBGASSERT(...)    _NoDbg_Sink_::NOOP();
#define DBGASSERT_IF(...) _NoDbg_Sink_::NOOP();

#define DBGTHROW(msg)     _NoDbg_Sink_::NOOP();
#define DBG_throw(x)      throw x
#define DBGABORT          _NoDbg_Sink_::NOOP();

#define DBGRAW            _NoDbg_Sink_{}<<
#define DBG               _NoDbg_Sink_{}<<
#define DBGDUMP           _NoDbg_Sink_{}<<
#define DBGBOX            _NoDbg_Sink_{}<<
#define DBG_(x)           _NoDbg_Sink_::NOOP();

#define DBGTRACE
#define _
#define DBGTRACE_QUIET    _NoDbg_Sink_::NOOP();
#define DBGTRACE_LOUD     _NoDbg_Sink_::NOOP();
#define DBGMARK
#define DBGBEEP

#define DBGLOG            _NoDbg_Sink_{}<<
#define DBGLOG_(x)        _NoDbg_Sink_::NOOP();
#define DBGLOGX(f)        _NoDbg_Sink_{}<<
#define DBGLOGX_(...)     _NoDbg_Sink_{}<<
#define DBGLOG_OPEN(x)	  _NoDbg_Sink_::NOOP();
#define DBGLOG_CLOSE(x)	  _NoDbg_Sink_::NOOP();

// ==========================================================================
//
#else //                        * DEBUG ON *
//
// ==========================================================================

#define DBG_ENABLE    { Dbg::Out::enable(); }
#define DBG_DISABLE   { Dbg::Out::disable(); }

//#define DBG_CFG_REPLACE_IOSTREAM

#if defined(DBG_CFG_REPLACE_IOSTREAM) && defined(_MSC_VER)
#  define _CRT_SECURE_NO_WARNINGS // To the person who prefers this to have been left enabled: sorry!...
   // But... If any MSVC header has already been included, the above is ineffective!... :-o
#  pragma warning(disable: 4996) // Also: #pragma warning(suppress : 4996) does nothing if wrapped in #ifdef _MSC_VER!... :))
#endif

//!! I wish this switching could be done at a better place!
#ifdef DBG_CFG_REPLACE_IOSTREAM
#  define _DBG_IONS sz
#else
#  define _DBG_IONS std
#endif

// Generic helpers
#define _DBG_PREPROC_GLUE_DIRECT_(X,Y) X##Y
#define _DBG_PREPROC_GLUE_(X,Y) _DBG_PREPROC_GLUE_DIRECT_(X,Y)

#if !defined(DBG_CFG_NO_WIN32) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__))
#define DBG_HAS_WIN32__
#endif

#if defined(DBG_HAS_WIN32__) && !defined(WINAPI) //&& defined (_MSC_VER)
#  ifndef _WINDOWS_ //!!?? Is this the canonical flag for including it (or _INC_WINDOWS)?
#    ifndef _MSC_VER
#      warning "Windows.h was not included, doing it with WIN32_LEAN_AND_MEAN, NOMINMAX, VC_EXTRALEAN... (Better to do it beforehand, to keep full control over how and when exactly it's done!)"
#    else
#      pragma message("Windows.h was not included, doing it with WIN32_LEAN_AND_MEAN, NOMINMAX, VC_EXTRALEAN... (Better to do it beforehand, to keep full control over how and when exactly it's done!)")
#    endif
#    undef  WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#    undef  VC_EXTRALEAN
#    define VC_EXTRALEAN
#    undef  NOMINMAX
#    define NOMINMAX
#    include <windows.h>
#    ifdef _MSC_VER
#      pragma comment(lib, "user32")  // help with using the Win32-specific features
       //!! But... this is only for MSVC! How to "autolink" with GCC and CLANG?! -> #5!
#    endif
#  endif
#endif

// #include <iostream> -> See later, at #if... DBG_CFG_REPLACE_IOSTREAM!
#include <sstream>
#include <fstream>
#include <stdexcept> // uncaught_exception
#include <vector>    // for trace paths & supporting 'DBGDUMP vector'
#include <string>    // for trace paths & supporting 'DBGDUMP string'
#include <string_view>
#include <cstdio>    // sprintf
#include <mutex>
#include <type_traits>

#ifndef DBG_CFG_STREAMPREFIX
#define DBG_CFG_STREAMPREFIX  "DBG> "
#endif

#ifndef DBG_CFG_DBGCONPREFIX
#define DBG_CFG_DBGCONPREFIX  "DBG> "
#endif

#ifndef DBG_CFG_MSGBOXPREFIX
#define DBG_CFG_MSGBOXPREFIX  ""
#endif

#ifndef DBG_CFG_TRACELINE_PREFIX
#define DBG_CFG_TRACELINE_PREFIX "";
#endif

#ifndef DBG_CFG_SCOPEINDENT_CHAR
#define DBG_CFG_SCOPEINDENT_CHAR ' '
#endif

#ifndef DBG_CFG_DEVICE
#define DBG_CFG_DEVICE           Dbg::Out::STD_STREAM
//#define DBG_CFG_DEVICE           Dbg::Out::DEBUG_CONSOLE
#endif

#ifndef DBG_CFG_TRACEDEVICE
#  define DBG_CFG_TRACEDEVICE DBG_CFG_DEVICE
//#  define DBG_CFG_TRACEDEVICE      Dbg::Out::STD_STREAM
//#  define DBG_CFG_TRACEDEVICE      Dbg::Out::DEBUG_CONSOLE
#endif

#ifndef DBG_CFG_OUTSTREAM
#define DBG_CFG_OUTSTREAM     _DBG_IONS::cerr
#endif

#ifndef DBG_CFG_TRACESTREAM
#define DBG_CFG_TRACESTREAM      DBG_CFG_OUTSTREAM
#endif

//===========================================================================
#ifndef DBG_TRACE_OFF
#  ifdef DBG_TRACE_SILENT
#    define DBGTRACE Dbg::Scope _DBG_PREPROC_GLUE_(_dbgscopetracer__,__LINE__) (__func__, __LINE__, Dbg::Scope::TRACE_ONLY);
     // Note: can't wrap these in {} to protect from accidental name clashes, because they
     // need to live through the whole enclosing scope for holding the stack frame for tracing!
#  else
#    define DBGTRACE Dbg::Scope _DBG_PREPROC_GLUE_(_dbgscopetracer__,__LINE__) (__func__, __LINE__, Dbg::Scope::PRINT);
#  endif
#  define DBGTRACE_QUIET { Dbg::Tracer::instance().off(); }
#  define DBGTRACE_LOUD  { Dbg::Tracer::instance().on(); }
#else
#  define DBGTRACE
#  define DBGTRACE_QUIET {0;}
#  define DBGTRACE_LOUD  {0;}
#endif

// For convenience:
#define _ DBGTRACE


#define DBGTHROW(msg) \
	{ Dbg::Out::throwtrace(#msg, __func__, __FILE__, __LINE__);	\
	  Dbg::Tracer::trigger_stack_dump();	\
	  throw std::exception(msg); }

#define DBG_throw(x) \
	{ Dbg::Out::throwtrace(x, #x, __func__, __FILE__, __LINE__);	\
	  Dbg::Tracer::trigger_stack_dump();	\
	  throw x; }

#define DBGASSERT(x) \
	Dbg::Asserter(__FILE__, __LINE__, __func__).verify((x) != 0, "CONDITION: \"" #x "\"")

#define DBGASSERT_IF(implication) DBGASSERT(implication : true)

#define DBGABORT \
	{ DBGRAW "*** ABORTING... (at ", __func__, ": ", __LINE__, ", ", __FILE__, ")"; \
	  Dbg::Tracer::instance().dump_call_stack(); /* Note: trigger_stack_dump is not enough for abort()! */	\
	  std::abort(); }

#define DBGRAW  Dbg::Out::raw_line() <<
#define DBG     Dbg::Out::dumper_line(__func__, 0) <<
#define DBGDUMP Dbg::Out::dumper_line(__func__, __LINE__, 0, {.decorate_cstr = true}) <<

#define DBGBOX  Dbg::Out::raw_line(Dbg::Out::FlushDbgLine(Dbg::Out::databuf(), Dbg::Out::POPUP_MESSAGE, Dbg::Out::dataout())) <<

#define DBG_(x) Dbg::Out::dumpval(#x, x);

#define DBGMARK { DBGRAW "--> " << __func__ << " (" << __FILE__ << ", " << __LINE__ << ")"; }

#ifdef DBG_HAS_WIN32__
#define DBGBEEP { MessageBeep((UINT)-1); } // Not ::MessageBeep; see #18!
#else
#define DBGBEEP { DBG_CFG_TRACESTREAM << '\a'; } // Even more of a hit and miss, though...
#endif

//===========================================================================
//	UTILITIES
//===========================================================================

//!! Alas, StringStream still uses <string>, which will probably end up including everything anyway...:
#include <string>
#include <utility> // forward, decay
#include <concepts>
#include <type_traits>
namespace std { class ios_base; } // Just to accept & ignore them!
namespace sz
{
//////////////////////////////////
class File
{
	FILE* f_ = 0;
	bool autoclose = true;
	// Reuse existing handle (intended for stdin/stdout/stderr, mostly!):
protected:
	explicit File(FILE* h) : f_(h), autoclose(false)  {}
public:
	explicit File(const char* name, const char* flags = "rb") { f_ = fopen(name, flags); }
	~File() { if (autoclose && f_) fclose(f_); }
	File(const File&) = delete;
	operator bool () const { return f_ && !ferror(f_) && !eof(); }
	bool eof() const { return f_ ? feof(f_) : false; }
	virtual std::size_t write(const char* buf, long long size) { //! std::streamsize count (~ssize_t)
		return handle() ? fwrite(buf, size, 1, handle()) : 0;
	}
	void flush() { if (f_) fflush(f_); } // fflush(0) would flush every open file! :)
	FILE* handle() const {
#ifdef assert
		assert(f_);
#endif
		return f_; }
};

//////////////////////////////////
class Stream : public File
{
public:
	Stream() : File((FILE*)0) {} // No file backing (IO-less stream); e.g. StringStream also relies on this.
	Stream(FILE* f) : File(f) {}

	enum fmtflags : unsigned {
		dec = 1, bin = 2, hex = 4, oct = 8, basefield = bin|oct|dec|hex,
		showbase,
	} flags_;
	fmtflags flags() const { return flags_; }
	fmtflags flags(fmtflags f) { fmtflags tmp = flags_; flags_ = f; return tmp; }
	fmtflags setf (fmtflags f) { return flags(f); }
	fmtflags setf (fmtflags f, fmtflags mask) {
		unsigned _setbits = static_cast<unsigned>(f);
		unsigned _mask    = static_cast<unsigned>(mask);
		return flags(fmtflags( (flags_ & ~_mask) | (_setbits & _mask) ));
	}
};

// Just "semantic placeholders" yet (StringStream derives from both, like std::stringstream):
class OStream : virtual public Stream { public: using Stream::Stream; };
class IStream : virtual public Stream { public: using Stream::Stream; };

//////////////////////////////////
template <typename T, typename = void> struct _format
{
	static constexpr const char* fmt = "%s";
	static constexpr const char* conv(T) { return "<<<UNKNOWN TYPE>>>"; }
};

#define _DEF_STREAM_TRAIT_(Type, ConvArgT, Fmt, PrintT, Expr) template <> struct _format<Type> { \
	static constexpr const char* fmt = Fmt; \
	static constexpr PrintT conv([[maybe_unused]] ConvArgT x) { return static_cast<PrintT>(Expr); } }

// Extend as needed...:
_DEF_STREAM_TRAIT_(const char*,   const char*,   "%s",  const char*, x);
_DEF_STREAM_TRAIT_(char*,         char*,         "%s",  const char*, x);
_DEF_STREAM_TRAIT_(char,          char,          "%c",  char, x);
_DEF_STREAM_TRAIT_(int,           int,           "%d",  int,  x);
_DEF_STREAM_TRAIT_(long,          long,          "%ld", long, x);
_DEF_STREAM_TRAIT_(unsigned,      unsigned,      "%u",  unsigned, x);
_DEF_STREAM_TRAIT_(unsigned long, unsigned long, "%lu", unsigned long, x);
_DEF_STREAM_TRAIT_(bool,          bool,          "%d",  int, x ? 1 : 0); //!! OR: "%c",  char, { return x ? 'T' : 'F'; });
_DEF_STREAM_TRAIT_(float,         float,         "%g",  double, x);
_DEF_STREAM_TRAIT_(double,        double,        "%g",  double, x);
_DEF_STREAM_TRAIT_(const void*,   const void*,   "%p",  const void*, x);
_DEF_STREAM_TRAIT_(void*,         const void*,   "%p",  const void*, x);
#undef _DEF_STREAM_TRAIT_

// Disable the simple formatting method for std::string:
template <> struct _format<std::string> { static constexpr void* fmt = nullptr; };

// Traits to detect iomanip-like functions...
//using IOManip = Stream& (*)(Stream&);
// Helper: function ptr (0 or more args)?
template<typename T> struct is_function_ptr : std::false_type {};
template<typename Ret, typename... Args> struct is_function_ptr<Ret(*)(Args...)> : std::true_type {};
// iomanip?
template<typename T, typename = void> struct is_iomanip : std::false_type {};
template<typename T> struct is_iomanip<T, std::void_t<decltype(std::declval<T&>()(std::declval<std::ios_base&>()))>> : std::true_type {};
// Functor-style?
template<typename T> struct is_iomanip<T, std::enable_if_t<is_function_ptr<T>::value
	&& std::is_same_v<decltype(std::declval<T>()()), void>>> : std::true_type {};

// Formatting spec. for various iomanips types:
template<typename T> struct _format<T, std::enable_if_t<is_iomanip<T>::value || is_function_ptr<T>::value>> {
	static constexpr const char* fmt = "%s";
	static constexpr const char* conv(auto&&) { return "<IOmanip>"; }
};
//////////////////////////////////
template <typename T, typename = std::enable_if_t<(_format<std::decay_t<T>>::fmt != nullptr)>>
	requires (!std::is_class_v<T> || !std::is_same_v<decltype(T::IOManip_Proxy), const bool>)
OStream& operator << (OStream& out, T&& x) {
	using DecayedT = typename std::decay_t<T>;
	char tmp[256]; // Adjust as needed...
	int len = snprintf(tmp, sizeof(tmp),
		_format<DecayedT>::fmt,
		_format<DecayedT>::conv(std::forward<T>(x))); // Writes size - 1, appends EOS!
	out.write(tmp, len); // std::string doesn't need the EOS, which len excludes.
	return out;
}

inline OStream& operator << (OStream& out, const std::string& x) { out.write(x.data(), x.size()); return out; }

//////////////////////////////////
class StringStream : public OStream, public IStream
{
public: //!! Had to move the op<< out from the class to not be shadowed by the free-func Stream << ops!
	std::string buffer;
public:
	const std::string& str() const { return buffer; }
	void str(const std::string&  s) { buffer = s; }
	void str(      std::string&& s) { buffer = s; }
	void clear() { buffer.clear(); }
	std::size_t write(const char* buf, long long size) override { buffer.append(buf, size); return size; }
};

/////////////////////////////////
//!! Custom Hex, Dec, ...
template <typename StreamT> class SetFlags
{
protected:
	using FlagsType = StreamT::fmtflags;
	FlagsType old_flags_{};
	StreamT* stream_ = nullptr;
	virtual void _set(FlagsType f) { stream_->setf(f); }
	// _reset() may get called twice, so it must be idempotent!
	virtual void _reset() { if (stream_) stream_->setf(old_flags_); }
public:
	void set(StreamT& stream, FlagsType f) { stream_ = &stream; old_flags_ = stream_->flags(); _set(f); }
	void set(StreamT& stream) { set(stream, FlagsType{}); }
	virtual ~SetFlags() { _reset(); }
};

template <class StreamT, typename T = void*>
struct bHex : public SetFlags<StreamT> {
	void _set(SetFlags<StreamT>::FlagsType) override {
		this->stream_->setf(StreamT::hex, StreamT::basefield);
		this->stream_->setf(StreamT::showbase);
	}
};

template <class StreamT, typename T = void*>
struct bDec : public SetFlags<StreamT> {
	void _set(SetFlags<StreamT>::FlagsType) override {
		this->stream_->setf(StreamT::dec, StreamT::basefield);
	}
};
/*!!
template <class StreamT, typename T = void*>
struct bBin : public SetFlags<StreamT> {
	void _set(SetFlags<StreamT>::FlagsType) override { ... }
};
!!*/

// Default identity converter for StreamRef instances
template <class StreamT, typename = void*>
struct Noop : public SetFlags<StreamT> {};


	template <class StreamT, class ScopedManip = sz::Noop<StreamT>>
	struct StreamRef {
		StreamT& s_;
		ScopedManip scoped_state_;

		StreamRef(StreamT& s) : s_(s) { scoped_state_.set(s); }
//!!??		StreamRef(const StreamRef& other) = delete; //!! Doesn't compile! :-/
		// Fortunately, it still works with copying, too, because the << chaining happens to kinda fake RAII:
		// the StreamRef chain will also die at the end of the expression, just like a temporary would! :)
		// (Well, provided that gaps in the brittle f*ing op<< definitions don't prematurely break that chaining. :-/ )
		StreamRef(const StreamRef& other) = default;
//!! DON'T:	operator StreamT&() { return s_; } // Now, this could be a fine tool for that premature chain-breaking! :)
	};

	// Param-capturing one-shot manip. proxy...
	template <template<class, typename> class Manip, typename T = void*>
	struct Proxy {
		static constexpr bool IOManip_Proxy = true;
		T value;
		Proxy(T x) : value(x) {}
		template <class StreamT> auto apply(StreamT& s) {

			static_assert(!std::is_same_v<T, void*>);

			Manip<StreamT, T> b;
			b.set(s); // bHex's set() is its own "apply"...

			return StreamRef<StreamT, Manip<StreamT, T>>(s) << value;
		}
	};
	// Parameterless scoped-manip. proxy...
	template <template<class, typename> class Manip>
	struct Proxy<Manip, void*> {
		static constexpr bool IOManip_Proxy = true;
		template <class StreamT> auto apply(StreamT& s) {
			return StreamRef<StreamT, Manip<StreamT, void*>>(s);
		}
	};

template <typename T = void*>
auto        Hex(T x) { return Proxy<bHex, T>(x); }
auto inline Hex()    { return Proxy<bHex>(); }
template <typename T = void*>
auto        Dec(T x) { return Proxy<bDec, T>(x); }
auto inline Dec()    { return Proxy<bDec>(); }


// StreamRef << T
//template <typename StreamT, typename T, typename U, template<class, typename> class ScopedManip = sz::Noop>
//auto  operator << (sz::StreamRef<StreamT, ScopedManip<StreamT, T>>&& sr, U x) { sr.s_ << x; return sr; }
// StreamRef << T&&
//!! This became ambiguous -- only in MSVC (19.40) --, with the const& version below, after #21! :-o
//!!?? (Or was it bad already before that, just missed to test it?!)
//template <typename StreamT, typename T, typename U, template<class, typename> class ScopedManip = sz::Noop>
//auto  operator << (sz::StreamRef<StreamT, ScopedManip<StreamT, T>>&& sr, U&& x) { sr.s_ << x; return sr; }
// StreamRef << const T&
template <typename StreamT, typename T, typename U, template<class, typename> class ScopedManip = sz::Noop>
auto  operator << (sz::StreamRef<StreamT, ScopedManip<StreamT, T>>&& sr, const U& x) { sr.s_ << x; return sr; }

// StreamRef<scoped iomanip> << const T&
template <typename StreamT, typename T, template<class, typename> class ScopedManip = sz::Noop>
auto  operator << (sz::StreamRef<StreamT, ScopedManip<StreamT, void*>>&& sr, const T& x) { sr.s_ << x; return sr; }

// AnyStream << Custom IOManip. Proxy //!! Had to move them here from the global namespace for #21 (#40)! :-o
template <class StreamT, typename T = void*> auto  operator << (StreamT& s, sz::Proxy<sz::bHex, T>&& proxy) { return proxy.apply(s); }
template <class StreamT, typename T = void*> auto  operator << (StreamT& s, sz::Proxy<sz::bDec, T>&& proxy) { return proxy.apply(s); }
//!!...

} // namespace sz


//---------------------------------------------------------------------------
#ifdef DBG_CFG_REPLACE_IOSTREAM

namespace sz {
  // These get initialized before using any of the DBG... macros:
  static inline OStream cout{stdout}, cerr{stderr}; //!! Must happen before the first streamed write!
} // namespace sz
  using DBG_OSTREAM_TYPE = decltype(sz::cerr);
  using DBG_SSTREAM_TYPE = sz::StringStream;

#else // DBG_CFG_REPLACE_IOSTREAM

# include <iostream>
# include <iomanip>
  using DBG_OSTREAM_TYPE = decltype(std::cerr);
  using DBG_SSTREAM_TYPE = std::stringstream;

#endif // DBG_CFG_REPLACE_IOSTREAM
//---------------------------------------------------------------------------


static std::string DBG_trim(const std::string& s)
{
	if (s.empty()) return s;
	std::size_t beg = s.find_first_not_of(" \t");
	if (beg == std::size_t(-1)) return std::string("");
	std::size_t end = s.find_last_not_of(" \t");
	return std::string(s, beg, end - beg + 1);
}

// Try reading the failed ASSERT statement from the source file...
// Well, this is, of course, meaningful only when testing locally.
// But that DOES tend to happen a lot, when assertions fail, ain't it?
static std::string fetch_assert_line(const char* filename, unsigned line)
{
	std::string assertline("");
	std::ifstream source(filename);

	for (unsigned l = 0; source && l < line; ++l)
		std::getline(source, assertline);

	return DBG_trim(assertline);
}


//===========================================================================
/// The core output manager.
/// All debug output goes through a static singleton instance of this class.
//===========================================================================
namespace Dbg
{
	// "For C++ reasons", this won't compile if kept inside Out,
	// with that default member initializer:
	struct Out_Config
	{
		bool decorate_cstr = false; // Show const char* literals quoted?
	};

struct Out
{
	using Config = Out_Config;
	enum DeviceType { STD_STREAM, POPUP_MESSAGE, DEBUG_CONSOLE };

	Config cfg_; // Can be reconfigured by a call to instance(cfg).

	DeviceType        datadevice_;
	DeviceType        tracedevice_;
	// Specific stream targets for the ...device_ == STD_STREAM case:
	DBG_OSTREAM_TYPE* dataout_;  // normal data dump output
	DBG_OSTREAM_TYPE* traceout_; // trace-info output

	DBG_SSTREAM_TYPE outbuf_;   // buffer for normal output
	DBG_SSTREAM_TYPE tracebuf_; // buffer for trace output

	// These must be "forward-defined" to workaround a Digital Mars C++ bug:
	static DeviceType    datadevice()    { return  instance().datadevice_; }
	static DeviceType    tracedevice()   { return  instance().tracedevice_; }
	static DBG_OSTREAM_TYPE&  dataout()  { return  *(instance().dataout_); }
	static DBG_OSTREAM_TYPE&  traceout() { return  *(instance().traceout_); }
	static DBG_SSTREAM_TYPE&  databuf()  { return  instance().outbuf_; }
	static DBG_SSTREAM_TYPE&  tracebuf() { return  instance().tracebuf_; }

	// Line-buffering provides a single point (in the dtors) for output
	// control, so it can be easily turned on/off or redirected to
	// various output devices (e.g. MessageBox, OutputDebugString, ...).
	struct FlushLine
	{
		DBG_SSTREAM_TYPE&  linebuf_;
		DeviceType         device_;
		DBG_OSTREAM_TYPE&  outstream_;
		bool               NL_on_flush_;

		FlushLine(DBG_SSTREAM_TYPE& linebuf = Out::databuf(),
			DeviceType device = Out::datadevice(),
			DBG_OSTREAM_TYPE& outstream = Out::dataout(),
			bool NL_on_flush = true)
		: linebuf_(linebuf), device_(device), outstream_(outstream), NL_on_flush_(NL_on_flush)
		{
		}
		void flush();
	private:
		FlushLine& operator=(FlushLine&) = delete; // silence warning: "cannot generate op=..."
	};
	struct FlushDbgLine : FlushLine
	{
		using FlushLine::FlushLine;
		~FlushDbgLine();
	};
	struct FlushTraceLine : FlushLine
	{
		using FlushLine::FlushLine;
		~FlushTraceLine();
	};

	Out(DeviceType datadevice, DeviceType tracedevice, DBG_OSTREAM_TYPE& datastream, DBG_OSTREAM_TYPE& tracestream);

	Out() {} //? Why is this needed? I forgot...

	static void enable()   { instance(true, true); }
	static void disable()  { instance(true, false); }

	static Out& instance(bool set = false, bool new_enable_state = false, const Config* cfg = nullptr)
	{
		// Static singleton instance
		static Out s_instance(DBG_CFG_DEVICE, DBG_CFG_TRACEDEVICE, DBG_CFG_OUTSTREAM, DBG_CFG_TRACESTREAM);

		// Static singleton null-output instance
#ifdef DBG_CFG_REPLACE_IOSTREAM
		static DBG_OSTREAM_TYPE DBG_NULLSTREAM(0);
#else
		//https://stackoverflow.com/a/11826666/1479945
		//!!TODO: optimize accorting to:
		//!!https://stackoverflow.com/questions/11826554/standard-no-op-output-stream#comment84884827_11826666
		static_assert(std::is_base_of_v<DBG_OSTREAM_TYPE, std::ostream>);
		struct NullBuffer : public std::streambuf { int overflow(int c) { return c; } };
		static NullBuffer null_buffer;
		static std::ostream DBG_NULLSTREAM(&null_buffer);
#endif // DBG_CFG_REPLACE_IOSTREAM
		static Out s_silent_instance(Out::STD_STREAM, Out::STD_STREAM, DBG_NULLSTREAM, DBG_NULLSTREAM);

		// "Nestable" enable/disable:
		static int enabled = 1; // If positive: use the live instance!
		if (set) enabled += (new_enable_state ? 1 : -1);

		// Reconfig?
		//!! This simplistic logic here is a terribly brittle experimental hack, to support #17
		//!! (i.e. to disable quoting const char* literals for DBG, but enable for DBGDUMP):
		if (cfg) s_instance.cfg_ = *cfg; //! (Re)set only via the dump_line() calls of DBG / DBGDUMP!

		return enabled > 0 ? s_instance : s_silent_instance;
	}

	static Out& instance(const Config& cfg) { return instance(false, false, &cfg); }
		//! Called only from the dump_line() calls of DBG / DBGDUMP!


	static DBG_OSTREAM_TYPE& raw_line([[maybe_unused]] const FlushDbgLine& cl = FlushDbgLine(databuf(), datadevice(), dataout()))
	{
		return databuf();
	}
/*
	static ostream& raw_line_nonl( const FlushDbgLine& cl = FlushDbgLine(databuf(), datadevice(), dataout(), false) )
	{
		return databuf();
	}
*/
	static Out& dumper_line(const char* func, int line, [[maybe_unused]] const char* file = "",
	                           [[maybe_unused]] Config cfg = Config(),
	                           [[maybe_unused]] const FlushDbgLine& cl = FlushDbgLine(databuf(), datadevice(), dataout()) )
	{
		databuf() << indent();
		if (func || line) {
			databuf() << "[";
			if (func) databuf() << func;
			if (func && line) databuf() << ":";
			if (line) databuf() << line;
			databuf() << "]: ";
		}
		return instance(cfg); //! Not the raw databuf() directly!
	}

	static DBG_OSTREAM_TYPE& trace_line([[maybe_unused]] const char* func = "",
	                                    [[maybe_unused]] int line = 0,
	                                    [[maybe_unused]] const char* file = "",
	                                    [[maybe_unused]] const FlushTraceLine& cl = FlushTraceLine(tracebuf(), tracedevice(), traceout()) )
	{
		tracebuf() << DBG_CFG_TRACELINE_PREFIX;
		return tracebuf();
	}

	static void throwtrace(const char* xstr, const char* func, const char* file, int line)
	{
		// +1 space after `indent` to fake a 2-space Tab...
		raw_line() << indent()<<" " << "!!!!! EXCEPTION: " << xstr;
		raw_line() << indent()<<" " << "             at: " << func << " (" << file << ", line: " << line << ")";
	}

	template <typename X>
	static void throwtrace(const X& x, const char* xstr, const char* func, const char* file, int line)
	{
		const char* what = "";
		if constexpr (std::is_base_of_v<std::exception, std::decay_t<X>>) { what = x.what(); }

		// +1 space after `indent` to fake a 2-space Tab... :)
		raw_line() << indent()<<" " << "!!!!! EXCEPTION: " << xstr << (*what?" // what(): \"":"")<< what <<(*what?"\"":"");
		raw_line() << indent()<<" " << "             at: " << func << " (" << file << ", line: " << line << ")";
	}

	template<typename T>
	static void dumpval(const char* x_label, T&& x)
	{
		instance() << std::forward<T>(x);
		std::string s = instance().databuf().str();
		instance().databuf().str(""); //! .clear() would clear the error bits!... Hilarious.
		if constexpr (std::is_same_v<std::decay_t<T>, const char*>)
			raw_line() << indent() << x_label;
		else
			raw_line() << indent() << x_label <<"\t: "<< s;
	}

	static const char* indent();

private:
	Out(const Out&) = delete;
	Out& operator=(const Out&) = delete;
}; // Out


//===========================================================================
/// op<< and op, pairs for printing via the Output Manager (see above)
///
///	- `op<<` is basically the usual << output with some extra
///	  display-decoration (depending on the type)
///
///	- `op,` prints a comma before printing the object
///
//===========================================================================
/// Comma op. doing raw (undecorated) printing like the original << of the type.
template <class T> DBG_OSTREAM_TYPE& operator , (DBG_OSTREAM_TYPE& o, T x)  { return o << x; }

/// Generic comma op. and << op. for decorated printing.
template <class T> Out& operator << (Out& d, T x) { Out::databuf() << x; return d; }
template <class T> Out& operator ,  (Out& d, T x) { Out::databuf() << ", "; return d << x; }

#if defined(_MSC_VER)
// VC++ does not prepend 0x to pointer values by default, so let's do it
// ourselves. ("<< ios_base::showbase" would be nicer, but would require
// saving/restoring the ios state...)
template <class T> Out& operator << (Out& d, T* x)
	{ if (x) Out::databuf() /*<<ios_base::showbase*/ << "0x" << x; else Out::databuf() << "NULL";
	  return d; }
#else
template <class T> Out& operator << (Out& d, T* x)
	{ if (x) Out::databuf() << (void*)x; else Out::databuf() << "NULL";
	  return d; }
#endif
// (Note: Digital Mars C++ seems to be unable to find this one.)
template<> inline Out& operator << (Out& d, const char* x)
	{ if (x) { if (d.cfg_.decorate_cstr) Out::databuf() << '\"' << x << '\"';
		   else                      Out::databuf() << x;
	  } else {                           Out::databuf() << "NULL"; }
	  return d; }

template<> inline Out& operator << (Out& d, char x)
	{ using ::sz::operator<<;
	  Out::databuf() << '\'' << x << '\'' <<" ("<< int(x) <<", "<<sz::Hex()<< int(x) <<")";
	  return d; }

template<> inline Out& operator << (Out& d, std::string x)      { Out::databuf() << '\"' << x << "\"s";  return d; }
template<> inline Out& operator << (Out& d, std::string_view x) { Out::databuf() << '\"' << x << "\"sv"; return d; }

template<> inline Out& operator << (Out& d, std::vector<int> x)	{
	Out::databuf() << "<";
	for (unsigned i=0; i<x.size(); ++i) { // Yeah, no, std::size_t is an "unjustified external dependency" here. :)
		Out::databuf() << x[i];
		if (i<x.size()-1) Out::databuf() << ", ";
	}
	Out::databuf() << ">";
	return d;
}


struct Scope;
struct ScopeInfo { const char* func; int line; };

//===========================================================================
/// The scope ("stack") tracer
struct Tracer
{
        //!! Can't properly do this without a .cpp + .obj... :-/ (Until porting to C++ modules?)
//!!	static inline std::terminate_handler app_terminate_handler = 0; // Must save and call it if set by the app!...

	Tracer()
	{
		trace_quietly_ = false;
		dumping_ = false;
		nesting_level_ = 0;
		indent[0] = '\0';
		call_stack.reserve(MAXNESTLEVEL);
		/*!! Since this won't work, we'll just refuse to register our own terminate handler if there's one already.
		if (!app_terminate_handler)
		     app_terminate_handler = std::set_terminate(&dbg_terminate_handler);
		// Prevent accidentally chaining to a previously set itself (e.g. with an `inline` app_handler var
		// with inconsistent ghost instances, dunno)...
		if (app_terminate_handler == &dbg_terminate_handler)
		    app_terminate_handler = 0;
		!!*/
		if (std::get_terminate()) {
			DBGRAW "WARNING: terminate_handler already set, not registering a DBG stack tracer (again).";
		} else {
		        std::set_terminate(dbg_terminate_handler);
			DBGRAW "NOTE: DBG terminate handler registered for stack tracing.";
		}
	}
	~Tracer()
	{
		if (dumping_) dump_call_stack();
	}
	static Tracer& instance()
	{
		static Tracer s_instance; // static singleton instance
		return s_instance;
	}

	void enter_scope(const Scope& scope);
	void leave_scope(const Scope& scope);
	static void trigger_stack_dump()
	{
		instance().dumping_ = true;
//!!		instance().dump_call_stack();
	}

	void dump_call_stack();

	// Run-time output control...
	void off() { trace_quietly_ = true; }
	void on()  { trace_quietly_ = false; }
	bool quiet() { return trace_quietly_; }

	// State...
	static const int MAXNESTLEVEL = 50;
	bool trace_quietly_;
	bool dumping_;
	int  nesting_level_;
	char indent[MAXNESTLEVEL];
	std::vector<Dbg::ScopeInfo> call_stack;

private:
	/// The "global" terminate-handler to catch unhandled exceptions etc.
	/// (Must be a member function to allow cross-module linking. :-/ )
	static void dbg_terminate_handler();
};
//!! Can't do this without a .cpp + .obj... :-/ (Until porting to C++ modules?)
//!!std::terminate_handler Dbg::Tracer::app_terminate_handler = 0;

struct Scope
{
	enum TraceMode {TRACE_ONLY, PRINT};
	TraceMode mode;
	ScopeInfo info;

	Scope(const char* f, int l, TraceMode m = TRACE_ONLY) {
		mode = m;
		info.func = f; info.line = l;
		Tracer::instance().enter_scope(*this);
	}
	~Scope() {
		Tracer::instance().leave_scope(*this);
	}
};

//------------------------
inline Out::FlushDbgLine::~FlushDbgLine()
{
//!!	if (Out::instance().quiet())
//!!		linebuf_.str("");
//!!	else
		flush();
}

// This one needs to be moved after Tracer in order to compile...
inline Out::FlushTraceLine::~FlushTraceLine()
{
	if (Tracer::instance().quiet())
		linebuf_.str("");
	else
		flush();
}

inline void Out::FlushLine::flush()
{
	if (NL_on_flush_) { linebuf_.write("\n", sizeof("\n")-1); } // Also compatible with sz::OStream (unlike << std::endl)!

	std::string cooked_line;
	switch (device_)
	{
	case POPUP_MESSAGE:
		#ifdef DBG_HAS_WIN32__
		#ifdef DBG_CFG_MSGBOXPREFIX
		cooked_line += DBG_CFG_MSGBOXPREFIX;
		#endif
		cooked_line += linebuf_.str();
		MessageBox(0, cooked_line.c_str(), "DEBUG MESSAGE",
				MB_ICONASTERISK & MB_ICONINFORMATION); // Not ::MessageBox; see #18!
		break;
		#endif
		; // Fall back to a debug console if no popup support...

	case DEBUG_CONSOLE:
		#ifdef DBG_HAS_WIN32__
		#ifdef DBG_CFG_DBGCONPREFIX
		cooked_line += DBG_CFG_DBGCONPREFIX;
		#endif
		cooked_line += linebuf_.str();
		OutputDebugString(cooked_line.c_str()); // Not ::OutputDebugString; see #18!
		break;
		#endif
		; // Fall back to std. stream output if not supported...

	default:
	case STD_STREAM:
		outstream_ << DBG_CFG_STREAMPREFIX << linebuf_.str();
		break;
	}

	linebuf_.str(""); // clear the line buffer
}
//---------------------------------------------------------------------------
// Note: relying solely on the destructors doing the dump-related work
// simply will not work, because the program may be abort()-ing, when
// no dtors would get called at all.

inline void Tracer::enter_scope(const Scope& scope)
{
	call_stack.push_back(scope.info);
	if (scope.mode == Scope::PRINT) Out::trace_line() << indent << "{ " << scope.info.func << ":" << scope.info.line << "";
	if (nesting_level_ < MAXNESTLEVEL) ++nesting_level_;
	indent[nesting_level_-1] = DBG_CFG_SCOPEINDENT_CHAR; indent[nesting_level_] = '\0';
}
inline void Tracer::leave_scope(const Scope& scope)
{
	if (nesting_level_ > 0) --nesting_level_;
	indent[nesting_level_] = '\0';
	if (scope.mode == Scope::PRINT) Out::trace_line() << indent << "} " << scope.info.func;
	call_stack.pop_back();
}
inline void Tracer::dump_call_stack()
{
#ifndef DBG_TRACE_OFF
	Tracer& tr = Tracer::instance();
	Out::raw_line() << "  Backtrace:";
	auto i = tr.call_stack.size();
	if (i == 0) {
		Out::raw_line() << "  -- (empty)";
		return;
	}
	do {
		--i;
		Out::raw_line()
			<< "  -- "
			<< tr.call_stack[i].func
			<< " (line "
			<< tr.call_stack[i].line
			<< ")";
	} while (i != 0);

	dumping_ = false; // done (prevent the destructor dump again)
#endif
}

inline void Tracer::dbg_terminate_handler()
{
//	if (Tracer::app_terminate_handler) {
//	  	DBGRAW "*** CALLING THE APP's TERMINATE HANDLER...";
//		Tracer::app_terminate_handler();
//	} else {
		Tracer::instance().dump_call_stack();
		DBGRAW "*** TERMINATING...";
		exit(-1);
//	}
}

//===========================================================================
// This must come after the def. of Dbg::Tracer:
inline const char* Out::indent() { return Tracer::instance().indent; }


//===========================================================================
struct Asserter
{
	const char* file_;
	unsigned    line_;
	const char* func_;

	Asserter(const char* file, unsigned line, const char* func) :
		file_(file),
		line_(line),
		func_(func)
	{}

	// This function receives the ASSERT(...) argument(s) directly!
	void verify(bool valid, const char* comment = 0)
	{
		if (valid) return;

		Out::raw_line(); //!!?? Why not just a NL?! Can it be subtly different in non-stderr outputs?
		Out::raw_line()
			<< "*** ASSERTION FAILED!!! ***"
		// Location:
			<< " in " << func_ << "(), "<< file_ << ", line " << line_ << ":";
		// Show the failed condition / custom comment:
		std::string assertline = fetch_assert_line(file_, line_);
		if (comment) {
			Out::raw_line() << ">>> " << (assertline.empty() ? comment : assertline);
		} else if (!assertline.empty()) {
			Out::raw_line() << ">>> " << assertline;
		}

		Tracer::trigger_stack_dump();
		throw "DBG_X_ASSERT(!!...)";
	}
};
//===========================================================================

//!! OBSOLETE; should be reworked/removed:
// ---------------------------------- Internal utilities...
	#define _DBG_FORMAT_FLOAT_(_buf, x) (( sizeof(x)==sizeof(float)  ? std::sprintf(_buf, "%f", (x))	\
	                                     : sizeof(x)==sizeof(double) ? std::sprintf(_buf, "%g", (x))	\
	                                     :                             std::sprintf(_buf, "(DBG: unknown float type)")), buf)

// ----------------------------------
#ifndef DBG_CFG_NO_LOG

#if 0
  #include <cstdio>
  #include <stdarg.h>
  //
  // These log functions use an explicitly created/selected output.
  //
  typedef FILE* DBGLOG;	// If changing this, the functions need reworking!

  static void DBGLOG_WRITE(DBGLOG log, const char* fmt, ...)
  {
	va_list args; va_start(args, fmt);
	__DBG_WRITE_VARG(log, fmt, args);
  }

  static DBGLOG DBGLOG_OPEN(const char* logname)
  {
        FILE* log = fopen(logname, "a+");
	if (!log) DBGOUT("DBG: Failed to open log output, using OutputDebugString!");
        DBGLOG_WRITE(log, "============================= --- BEGIN LOG --- =============================");
        return log;
  }

  static void DBGLOG_CLOSE(DBGLOG log)
  {
        DBGLOG_WRITE(log, "============================== --- END LOG --- ==============================");
        fclose(log);
	// Let's be nice...:
	if (log == __dbg_output) {
		DBGOUT("DBG: Closing default output, switching to OutputDebugString!");
		DBGSETOUTPUT(0);	// 0 means OutputDebugString
	}
  }

  static void DBGFILE(const char* filename, const char* fmt, ...)
  {
        DBGLOG log;
        va_list args; va_start(args, fmt);

        log = DBGLOG_OPEN(filename);
        __DBG_WRITE_VARG(log, fmt, args);
        DBGLOG_CLOSE(log);
  }
#endif


struct DbgLog
// Simple, "stateless", low-volume debug logger
// NOTE: Despite explicit DBGLOG_OPEN / DBGLOG_CLOSE pair of macros,
//       each `DBGLOG a, b...` statement will also do an open/close!
//       (A trade-off for simplicity and robustness, at the cost of efficiency.)
{
	// Let's make it thread-safe, if we're at it...
	static std::mutex& mutex() { static std::mutex _s_mutex; return _s_mutex; }

	//!! This is actually pretty universal, not specific to debug logging (apart from the default filename):
	//!! Add switch to append/truncate!
	static std::ofstream& streamer(std::string_view filename = "", bool open = true) {
		static std::string _s_filename = !filename.empty() ? std::string(filename) : "debug.log";
		static std::ofstream _s_stream{}; // Will follow the changes of `filename`.

		if (!filename.empty() && filename != _s_filename) {
			if  (_s_stream.is_open()) _s_stream.close();
			_s_filename = filename;
		}
		if (open && !_s_stream.is_open()) _s_stream.open(_s_filename, std::fstream::app);
		return _s_stream;
	}

	// The non-static state it thread-safe, PROVIDED the class is ONLY used
	// via the DBGLOG* macros, which create temporaries!
	std::unique_lock<std::mutex> lock; // Lock the entire "line instance": got garbled output across op<< calls otherwise!
	                                   //!! Also, scoped_guard didn't work for some reason! :-o
	std::string filename_wanted_{};
	bool newline_at_close_{true};
	DbgLog(std::string_view fname_, bool nl = true)
		: lock(mutex()),
		  filename_wanted_(std::string(fname_)), newline_at_close_(nl) { streamer(filename_wanted_); }
	DbgLog() : lock(mutex()) {}
	DbgLog(const DbgLog&) = delete;
	~DbgLog() { if (newline_at_close_) DbgLog::streamer().write("\n", sizeof("\n")-1); close(); }
	void close() { streamer(filename_wanted_, false); } //! Yes, reopening, just to force-closing a possibly open *other* file!...

	template<typename T> DbgLog& operator << (T const& x) { DbgLog::streamer(filename_wanted_) << x; return *this; }
	template<typename T> DbgLog& operator ,  (T const& x) { DbgLog::streamer(filename_wanted_) << ", " << x; return *this; }
};

// Single-threaded (stateful) version:
#define DBGLOG_USE(f)  Dbg::DbgLog{f, false}
#define DBGLOG         Dbg::DbgLog{} << // Can be used as either `DBGLOG(f) a << b...` or `DBGLOG(f) a, b...`
#define DBGLOG_(x)     Dbg::DbgLog{} << (#x) << " = " << (x)

// Multi-threaded (stateless) (could as well be just a simple `ostream(f) <<`!...):
#define DBGLOGX(f)     Dbg::DbgLog{f} << // Can be used as either `DBGLOGX(f) a << b...` or `DBGLOGX(f) a, b...`
#define DBGLOGX_(f, x) Dbg::DbgLog{f} << (#x) << " = " << (x)

/*
//https://stackoverflow.com/a/3048361/1479945, https://stackoverflow.com/a/8814003/1479945
#define XXX_0()                     DBGLOG_ADD
#define XXX_1(f)                    DBGLOG_OPEN(f)
// The interim macro that simply strips the excess and ends up with the required macro
#define XXX_X(x,A,FUNC, ...)  FUNC
// The macro that the programmer uses
#define DBGLOG(...)                 XXX_X(,##__VA_ARGS__,\
                                    XXX_1(__VA_ARGS__),\
                                    XXX_0(__VA_ARGS__)\
                                    )
*/
#endif // DBG_CFG_NO_LOG



inline Out::Out(DeviceType datadevice, DeviceType tracedevice, DBG_OSTREAM_TYPE& datastream, DBG_OSTREAM_TYPE& tracestream)
:
	datadevice_(datadevice),
	tracedevice_(tracedevice),
	dataout_(&datastream),
	traceout_(&tracestream)
{
}

// ==========================================================================
//
//                                 * END *
//
// ==========================================================================
} // namespace Dbg

using Dbg::operator<<, Dbg::operator,;

#endif // DBG_OFF

// Need to define it here, after DBG has been defined...
//inline X_Invariant_Violation::X_Invariant_Violation(const char* expr)
//	{ DBG "*** Invariant violation (Internal Error: ", (expr ? expr : "UNKNOWN"), ")! ***"; }

#endif // _DBG_HH_NC249T87DN2376T27E64N5978Y35987_
