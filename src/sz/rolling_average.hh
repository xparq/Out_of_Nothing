#ifndef _ROLLING_AVG_HH_
#define _ROLLING_AVG_HH_

namespace sz {

template<int MAX_ITEMS, typename FloatT = float>
struct RollingAverage
{
	FloatT vq[MAX_ITEMS];
	short n;
	short tail;
	FloatT sum;

	RollingAverage() : vq{0}, n(0), tail(0), sum(0) {}
	void update(FloatT last) {
		sum += last;
		if (n == MAX_ITEMS) sum -= vq[tail]; else ++n;
		vq[tail++] = last;
		tail %= MAX_ITEMS;
	}
	operator FloatT() { return sum / n; }
	operator int()    { return (int)(sum / n); }
};

}; // namespace

#endif _ROLLING_AVG_HH_
