#ifndef _SARTITABTFUYTFVYIDTSUYATSUIYASTD7Q8E7T8SDT7_
#define _SARTITABTFUYTFVYIDTSUYATSUIYASTD7Q8E7T8SDT7_
         

#include "sys/file.hh"

#include <string>
#include <utility> // forward, decay
#include <concepts>
#include <type_traits>

namespace std { class ios_base; } // Just to accept & ignore them!
namespace sz
{
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


#endif _SARTITABTFUYTFVYIDTSUYATSUIYASTD7Q8E7T8SDT7_
