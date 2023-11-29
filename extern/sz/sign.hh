#ifndef _SZ_SIGN_HH_
#define _SZ_SIGN_HH_

namespace sz {

template <typename T>
T sign(T x) {
	// That weird "wishful casting" is a plea of "Please do no runtime conv. for these, like ever!..."
	// (Hopefully redundant, but might potentially still silence some warnings, somebody please tell.)
	if (x > 0) return (T)1;
	if (x < 0) return (T)-1;
	return (T)0;
}

}; // namespace
#endif _SZ_SIGN_HH_
