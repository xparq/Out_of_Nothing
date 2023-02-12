template<int MAX>
struct RollingAverage
{
	float vq[MAX];
	short n;
	short tail;
	float sum;

	RollingAverage() : vq{0}, n(0), tail(0), sum(0) {}
	void update(float last) {
		sum += last;
		if (n == MAX) sum -= vq[tail]; else ++n;
		vq[tail++] = last;
		tail %= MAX;
	}
	operator float() { return sum / n; }
	operator int() { return (int)(sum / n); }
};
