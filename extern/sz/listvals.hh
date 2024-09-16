// 1.2.0
// =============================================================== INTERFACE {
//	Moderately versatile string list decorator function (template)
//
//	Supports containers that match a StringContainer concept defined
//	below; and "string" can be both std::string and const char*.
//	Returns the result as std::string.
//
//	Optionally and/or by default:
//	- Quotes strings containing whitespace, unless `quote` is set
//	  to null or empty.
//	- Separates items with commas.
//	- Wraps the list in prefix/postfix strings.

#include <type_traits>

// The StringContainer concept...
	// Helpers to check for existence of member functions and type properties:
	template<class, typename = void>struct has_size : std::false_type {};
	template<class T>
	struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};
	template<class, typename = void> struct has_begin_end : std::false_type {};
	template<class T> struct has_begin_end<T,
		std::void_t<
			decltype(std::declval<T>().begin()),
			decltype(std::declval<T>().end())
		>
	> : std::true_type {};

template<class T>
concept StringContainer =
	has_size<T>::value &&
	has_begin_end<T>::value &&
	requires(T t) {
		// Checks for string-like elements:
		requires sizeof(*std::declval<T>().begin()) >= sizeof(char*);
//!!??		requires std::is_same_v<const char*,        decltype(*t.begin())>;
//!!		requires std::is_same_v<const char* const*, decltype( t.begin())>
//!!		|| { t.begin()->length() };
	};


#include <string>
namespace sz
{
inline std::string listvals(StringContainer auto const& container,
	const char prewrap[] = "", const char postwrap[] = "", const char sep[] = ", ",
	// Not const char[], to hint that they accept nullptr:
	const char* quote = "\"",
	const char* chars_to_quote = " \t\n\r");
}
// =============================================================== } INTERFACE


//#ifdef SZ_IMPLEMENTATION //=================================================
//	Separating the IMPL. section is disabled, as this is a template
//	function (note that `auto`!), so its impl. must also be #included!

#include <string_view>
#include <cstring>
#include <cassert>

namespace sz
{
std::string listvals(StringContainer auto const& container,
                     const char prewrap[], const char postwrap[], const char sep[],
                     const char* quote, const char* chars_to_quote)
{
#define FOUND(expr) ((expr) != std::string_view::npos)
#define CONTAINS(str, chars) FOUND(std::string_view(str).find_first_of(chars))
	using namespace std;

	string result;
	if (!container.empty()) {
		auto QLEN = quote ? strlen(quote) : 0;
		// Precalc. size... (Note: we're processing cmd args. We got time.)
		auto size = strlen(prewrap) + (container.size() - 1) * strlen(sep) + strlen(postwrap);
		for (const auto& v : container) {
			if constexpr (is_same_v<decay_t<decltype(v)>, const char*>) {
				size += strlen(v);
			} else {
				size += v.length();
			}
			size += (quote && *quote && CONTAINS(v, chars_to_quote) ? // add quotes...
					(QLEN>1 ? QLEN:2) : 0); // special case for 1 (-> pair)!
		}
		result.reserve(size);
		// Write...
		result += prewrap;
		for (auto v = container.begin(); v != container.end(); ++v) {
			if (quote && *quote && CONTAINS(*v, chars_to_quote))
				{ result += string_view(quote, quote + (QLEN/2 ? QLEN/2 : 1)); // special case for 1 quote!
				  result += *v;
				  result += string_view(quote + QLEN/2); }
			else    { result += *v; }
			result += (++v == container.end() ? postwrap : sep); --v;
		}
		assert(result.length() == size);
	}
	return result;
#undef FOUND
#undef CONTAINS
}

} // namespace sz


//============================================================================
#ifdef UNIT_TEST

struct Cont {
	const char* const* data_;
	unsigned size_;

	template<unsigned N>
	Cont(const char* const (&data)[N]) : data_(data), size_(N) {}

	auto size()  const { return size_; }
	auto empty() const { return !size(); }
	auto begin() const { return data_; }
	auto end()   const { return data_ + size_; }
};

#include <vector>
#include <iostream>
int main()
{
	using namespace sz;
	using namespace std;

	const char* s[] = {"x", "y", "and z"};
	auto csa = Cont(s);
	cout << listvals(csa, "const char* -> [", "]") << '\n';

	vector<string> sa = {"x", "y", "and z"};
	cout << listvals(sa, "string -> [", "]") << '\n';
}

#endif // UNIT_TEST
