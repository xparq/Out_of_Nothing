#ifndef __SIGN_HPP__
#define __SIGN_HPP__

template <typename T>
T sign(T x) {
	// The weird "wishful casting" is like a prayer of "Please do no runtime conv. for these!..."
	// Hopefully redundant, but might potential still silence some warnings.
	if (x > 0) return (T) 1;
	if (x < 0) return (T) -1;
	return (T) 0;
}

#endif __SIGN_HPP__
