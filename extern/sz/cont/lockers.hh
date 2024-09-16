// 0.1.2
//#define DEBUG
/*
Constant-time "free-list" resource manager/allocator/container

	Clients can obtain opaque handles (keys) of "locker slots" (cells)
	from a managed pool of a fixed number of contiguous cells, as long
	as there are free ones left, and store a value of type T there.
	(The container itself is agnostic to that actual type, it's just
	a parameter to it.)

	Each cell represents a resource, and the key of the cell is
	meant to be used as a handle to that resource. Each key is unique
	in the context of the container.
	(In practice, often proxy resources (real OS handles, (smart)
	pointers, (weak) references, indexes etc.) would be stored in
	the cells.)

	The intended use case is to model resource allocation (i.e. "locked"
	exclusive use -- not in terms of concurrency, but ownership.
	Once a key has been obtained, another key to the same slot can't
	be obtained.

	Later, when done using the slot, its key should be "returned" to
	the container, so the cell be freed for reuse.
	(Subsequent use of the same cell is not guaranteed to have the same
	key assigned to it as before; actually, the notion of "same cell"
	isn't even defined: it's always just one isolated cell that can be
	accessed via a key, and there is no defined relation between keys.)

	Note: upon returning the key, the content of the cell will be
	implicitly disposed of (destructed), but whether that would also
	mean destroying (deallocating) the actual resource, is completely
	outside of the concern of the container.

	Accessing the cell contents (e.g. to store or retrieve a T cargo
	value) is done via the usual key-value map semantics. There is
	no ordering defined among the elements. There isn't even iterator
	support either (so, e.g. in the STL sense, this isn't even a
	container; see later). Copying the container (considered to be
	a resource manager) is disabled, too.

	Otherwise, the usual STL-like semantics applies: default-constructed
	empty cells, by-value copy on 'store' (via op=), the usual STL-like
	queries (empty, size, capacity etc.), no checks by default (making
	it the user's responsibility to keep things valid).

	No exceptions are thrown currently. Instead, the enum value ERROR
	is returned, when trying to obtain a slot while none is available.

NOTES:

 -  Rationale for not adding iterators:

    -	The keys are opaque, and the elements are not ordered, because in
	the target use case those elements live entirely independently, not
	forming a set that would (often) need to be manipulated as a whole.

	The owner of the container may still need to iterate through the
	elements occasionally, which can be done "manually", using the
	pool() query, and the guarantee that it's actually a sequential,
	contiguous storage of a known number of cells.

    -	Iterating over the *allocated* slots could be expensive. Note:
	just going through every *existing* (possibly free) cell makes
	little sense, and selecting the actually allocated ones would need
	an O(n) linear search *at every ++/-- operation* to find out
	if the next/prev. cell , *and* also repeating that to ignore
	empty slots -- in the worst case also ~n times, so we're somewhere
	near O(n*n) territory...
	(Keeping track of used slots, too, could make that constant time,
	but only at the cost of degrading the performance of free() to
	linear time (if I'm not mistaken, because removing a key from the
	'used' list requires another linear search, unlike allocation).
	A better approach could be an occupancy bit mask: that would keep
	the constness of the container ops., while also allowing const-time
	iterator inc./dec.)

    -	Sloppy client-side ownership management may lead to classic dangling-
	pointer mistakes (storing direct references, while missing that other
	code paths can free the same resource).
	By using handles, instead of direct T element references (pointers,
	STL-like iterators), the explicit indirection

	  a) triggers conscious effor (a.k.a "thinking") when accessing elements,

	  b) offers a checkpoint in the container to catch invalid references
	     (in DEBUG mode).
*/

#ifndef _3986GH78F786D102D75426748GB_
#define _3986GH78F786D102D75426748GB_

#include <cstdint> // size_t
#include <cassert>

namespace sz {

template <typename T, std::size_t MAX>
class lockers
{
public:
	using key_t = std::size_t; // (Could be some other int., preferably unsigned; !!NOT SUPPORTED YET!)

	enum : key_t { ERROR = key_t(-1) };

	//--------------------------------------------------------------------
	// Reserve/Store/Release...

	//!! Can't call this "reserve" for potential confusion to STL users... :-(
	[[nodiscard]] key_t alloc() { // Or return ERROR
		if (full()) return ERROR; // No assert, unlike for double free!
		else return _pop_free();
	}
	key_t store(const T& cargo) {
		if (key_t slot; ERROR == (slot = alloc())) return ERROR;
		else { store(slot, cargo); return slot; }
	}
	T& store(key_t slot, const T& cargo) {
		assert(slot != ERROR);
		assert(!_is_free(slot));
		return _slots[slot] = cargo; } // Support method-chaining
	void free(key_t slot) {
		assert(!empty());
		assert(slot != ERROR);
		assert(!_is_free(slot)); // Double free!
		_push_free(slot);
	}

	// Alloc/free synonyms (until I can decide which set to keep...):
	[[nodiscard]] key_t get()                { return alloc(); }
	void                release(key_t slot)  { free(slot); }

	//--------------------------------------------------------------------
	// Element access...

	const T& operator [](key_t key) const {
		assert(!empty());
		assert(key != ERROR);
		assert(!_is_free(key));
		return _slots[key]; }
	T& operator [](key_t key) {
		assert(!empty());
		assert(key != ERROR);
		assert(!_is_free(key));
		return _slots[key]; }

	//--------------------------------------------------------------------
	// Queries...

	bool empty()    const { assert(_end_free_i <= _capacity); return _end_free_i == _capacity; }
	bool full()     const { return _end_free_i == 0; }
	auto size()     const { return _capacity - _end_free_i; }
	auto capacity() const { assert(_capacity == MAX); // a) for now..., b) Unbelievable: fails with MSVC /DBDEBUG! :-o
	                        return _capacity; }

	const T* pool() const { return _slots; }
	      T* pool()       { return _slots; }

	bool is_free(key_t slot) const {
//std::cerr << "- checking key at is_free: " << slot << endl;
		assert(slot != ERROR);
		assert(slot >= 0);
		assert(slot < _capacity);
		//assert(!empty()); // Well, every key is free if there's nothing allocated... :)
		return _is_free(slot); }

	// Default ctor.
	lockers()
	{
		// We're working backwards from a "fake" full state:
		assert(full()); // "full" here just means we don't have the free items pool (stack) init'd yet.

		// Put all the keys we manage into the free-stack, rendering the container state "empty":
		for (key_t k = 0; !empty(); ++k)
			_push_free(k);

		assert(empty());
		assert(size() == 0);
		assert(_end_free_i == _capacity);

#ifdef DEBUG
		for (auto i = 0; i < _capacity; ++i) {
			assert(_free_stack[i] == i);
		}
		for (auto i = 0; i < _capacity; ++i) {
			_slots[i] = (T)'_';
		}
#endif		
	}

	lockers(const lockers&) = delete; // No copy!

/*
	// This is a limited, not-very-useful "iterator" offering for now, just
	// to support range-based for to go through *each existing* storage slot
	// (i.e. not each *actually used* one)!
	T*       begin()       { return &_slots[0]; }
	T*       end()         { return begin() + _capacity; }
	const T* begin() const { return &_slots[0]; }
	const T* end()   const { return begin() + _capacity; }
	const T* cbegin()      { return begin(); }
	const T* cend()        { return end(); }
	// Convert auto iterators provided by a range-for back to keys:
	key_t key(const T* it) const {
//std::cerr << "- key(): " << hex << (void*) it << " -> " << dec << (it - &_slots[0]) << endl;
		return it - &_slots[0]; }
*/


protected:
	//--------------------------------------------------------------------
	// Internals - Data...

	std::size_t  _capacity = MAX; //!! DYNAMIC GROWTH NOT YET IMPL.!
	T            _slots[MAX];
	key_t        _free_stack[MAX];
	std::size_t _end_free_i = 0; // Not key_t, as this is an internal iterator/index of keys! (Nice "illiteration", eh? ;) )

	//--------------------------------------------------------------------
	// Internals - Utilities...

	void _push_free(key_t k) {
		assert(!empty());
		_free_stack[_end_free_i++] = k;
#ifdef DEBUG
		_slots[k] = (T)'~'; // Mark the freed element (with some luck with T... :) )
#endif
	}
#ifndef DEBUG
	key_t _pop_free() {
		//!!assert(!full()); //!! Already returns ERROR at run-time! Check for that at the call sites instead!
		return full() ? ERROR : _free_stack[--_end_free_i];
	}
#else
	key_t _pop_free() { if (full()) return ERROR;
		auto slot = _free_stack[--_end_free_i];
		_free_stack[_end_free_i] = (key_t)ERROR; // Mark the unused index slot
		return slot;
	}
#endif
	bool _is_free(key_t slot) const {
		for (auto free_i = 0; free_i < _end_free_i; ++free_i) //! Not free_i < size()!...
			if (slot == _free_stack[free_i]) return true;
		return false;
	}

/*!! This unfinished draft would iterate the free slots -- which I forgot is utterly useless! :))

	struct safe_forward_iterator
	{
		safe_forward_iterator operator ++()    { return ++_freeslot_ptr; }
		safe_forward_iterator operator ++(int) { return _freeslot_ptr++; }
		safe_forward_iterator operator --()    { return --_freeslot_ptr; }
		safe_forward_iterator operator --(int) { return _freeslot_ptr--; }
		T& operator *()  { return _container->_slots[_freeslot_ptr*]; }
		T& operator ->() { return _container->_slots + _freeslot_ptr*; }

		bool operator ==(const safe_forward_iterator& other) { return _freeslot_ptr == other._freeslot_ptr; }
		safe_forward_iterator& operator =(const safe_forward_iterator& other)
			{	_container = other._container;
				_freeslot_ptr = other._freeslot_ptr;
				return *this; }

		// "Forward" in not really any useful meaning though, I guess!
		safe_forward_iterator(lockers* cont, lockers::_key_t* start = nullptr):
			_container(cont),
			_freeslot_ptr(start ? start : _container->_free_stack) // ... + _container->_end_free_i with reversed ++/--
			                                                       // could respect the stack ordering, but... pointless?
			{ assert(_container != nullptr); }
		safe_forward_iterator() = default;
	private:
		// Order matters!
		lockers* _container = nullptr;
		lockers::_key_t* _freeslot_ptr = nullptr;
	};

	using iterator = safe_forward_iterator;
!!*/
}; // class lockers
} // namespace sz


//----------------------------------------------------------------------------
#ifdef UNIT_TEST
#include <iostream>
using namespace std;
using namespace sz;

template <class T>
auto dump_state = [](const T& lockers) {
	for (auto slot = 0; slot < lockers.capacity(); ++slot) {
#ifdef DEBUG
//		cout << (char)lockers[slot]; // The DEBUG extensions have already marked the free items.
		                             // Umm... But... [] access is disallowed for freed items, so:
		cout << (lockers.is_free(slot) ? '_' : (char)(lockers[slot] > 31 ? lockers[slot] & 127 : '?')); // Yuck, is_free means another loop internally!
#else		
		cout << (lockers.is_free(slot) ? '_' : (char)(lockers[slot] > 31 ? lockers[slot] & 127 : '?')); // Yuck, is_free means another loop internally!
#endif
	}
	cout << " - occupied: " << lockers.size() << endl;
};

int main()
{
	using L = lockers<char, 4>;

	L slots;
	L::key_t a = -1, b = -2, c = -3, d = -4, err = -666;

//	slots.is_free('?'); // Not a key; should fail!

	assert(slots.empty());
	dump_state<L>(slots);

	a = slots.get();      assert(a != L::ERROR);
	b = slots.store('B'); assert(b != L::ERROR);
	c = slots.get();      assert(c != L::ERROR);
	d = slots.get();      assert(d != L::ERROR);

	assert(slots.full());
	err = slots.get();      assert(err == L::ERROR); // Now fail!
	err = slots.store('X'); assert(err == L::ERROR); // Ditto

	cout << slots.store(d, 'd') << '\n';
	cout << (slots[a] = 'a') << '\n';
	cout << "capacity: " << slots.capacity() << '\n';
	dump_state<L>(slots);

	cout << (slots[b] = 'b') << '\n';
	cout << (slots[c] = 'c') << '\n';

	cout << "Keys obtained (a, b, c, d, err): "<< a <<", "<< b <<", "<< c <<", "<< d << ", "<< err << '\n';
	dump_state<L>(slots);

	slots.free(a);
	cout << "free(a) -> is_free(a)? " << slots.is_free(a) << '\n';
	dump_state<L>(slots);

//	cout << (slots[a] = 'A') << '\n'; // DEBUG check should fail
//	dump_state<L>(slots);

	slots.store('Z'); // Should replace 'a'
	dump_state<L>(slots);
}
#endif // UNIT_TEST
#endif // _3986GH78F786D102D75426748GB_
