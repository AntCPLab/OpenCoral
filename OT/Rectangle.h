/*
 * Rectangle.h
 *
 */

#ifndef OT_RECTANGLE_H_
#define OT_RECTANGLE_H_

#include "Tools/random.h"
#include "OT/BitVector.h"
#include "Math/Z2k.h"

#define TAU(K, S) 2 * K + 4 * S
#define Y(K, S) X(TAU(K, S), K + S) X(K + S, K + S)

template <class U, class V>
class Rectangle
{
public:
	static const int N_ROWS = U::N_BITS;
	static const int N_COLUMNS = V::N_BITS;
	static const int N_ROW_BYTES = V::N_BYTES;

	V rows[N_ROWS];

	bool operator==(const Rectangle<U,V>& other) const;
	bool operator!=(const Rectangle<U,V>& other) const
			{ return not (*this == other); }

	bool get_bit(int i, int j) { return rows[i].get_bit(j); }

	Rectangle<U, V>& operator+=(const Rectangle<U, V>& other);
	Rectangle<U, V> operator-(const Rectangle<U, V> & other);

	template <class T>
	Rectangle<U, V>& rsub(Rectangle<U, V>& other) { return rsub_(other); }
	Rectangle<U, V>& rsub_(Rectangle<U, V>& other);
	template <class T>
	Rectangle<U, V>& sub(const void* other) { return sub_(other); }
	Rectangle<U, V>& sub_(const void* other);

	void mul(const BitVector& a, const V& b);

	void randomize(PRNG& G);
	template <class T>
	void randomize(int row, PRNG& G) { rows[row].randomize(G); }
	template <class T>
	void conditional_add(BitVector& conditions, Rectangle<U, V>& other,
			int offset) { conditional_add_(conditions, other, offset); }
	void conditional_add_(BitVector& conditions, Rectangle<U, V>& other,
			int offset);
	template <class T>
	void to(T& result);
	void to(Rectangle<U, V>& result) { result = *this; }

	void pack(octetStream& o) const;
	void unpack(octetStream& o);
};

template <int K, int L>
using Z2kRectangle = Rectangle<Z2<K>, Z2<L> >;

template <int K>
using Z2kSquare = Rectangle<Z2<K>, Z2<K>>;

#endif /* OT_RECTANGLE_H_ */
