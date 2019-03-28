/*
 * Z2k.h
 *
 */

#ifndef MATH_Z2K_H_
#define MATH_Z2K_H_

#include <mpirxx.h>
#include <string>
using namespace std;

#include "Tools/avx_memcpy.h"
#include "bigint.h"
#include "field_types.h"
#include "mpn_fixed.h"
#include "ValueInterface.h"

template <int K>
class Z2 : public ValueInterface
{
	template <int L>
	friend class Z2;
	friend class bigint;

	static const int N_WORDS = ((K + 7) / 8 + sizeof(mp_limb_t) - 1)
			/ sizeof(mp_limb_t);
	static const int N_LIMB_BITS = 8 * sizeof(mp_limb_t);
	static const uint64_t UPPER_MASK =
			((K % N_LIMB_BITS) == 0) ? -1 : -1 + (1LL << (K % N_LIMB_BITS));

	mp_limb_t a[N_WORDS];


public:
	typedef void Inp;
	typedef void PO;

	class DummyZ2kProtocol
	{
	public:
	    static void assign(Z2& _,  const Z2& __, int& ___)
        {
            (void) _, (void) __, (void) ___;
            throw not_implemented();
        }
	};
	typedef DummyZ2kProtocol Protocol;

	static const int N_BITS = K;
	static const int N_BYTES = (K + 7) / 8;

	static int size() { return N_BYTES; }
	static int t() { return 0; }

	static char type_char() { return 'Z'; }
	static string type_string() { return "Z2^" + to_string(int(N_BITS)); }

	static DataFieldType field_type() { return DATA_INT; }

	static const bool invertible = false;

	template <int L, int M>
	static Z2<K> Mul(const Z2<L>& x, const Z2<M>& y);

	typedef Z2<K> value_type;
	typedef Z2 next;

	Z2() { assign_zero(); }
	Z2(uint64_t x) : Z2() { a[0] = x; }
	Z2(__m128i x) : Z2() { avx_memcpy(a, &x, min(N_BYTES, 16)); }
	Z2(int x) : Z2(long(x)) { a[N_WORDS - 1] &= UPPER_MASK; }
	Z2(long x) : Z2(uint64_t(x)) { if (K > 64 and x < 0) memset(&a[1], -1, N_BYTES - 8); }
	Z2(const Integer& x);
	Z2(const bigint& x);
	Z2(const void* buffer) : Z2() { assign(buffer); }
	template <int L>
	Z2(const Z2<L>& x) : Z2()
	{ avx_memcpy(a, x.a, min(N_BYTES, x.N_BYTES)); normalize(); }

	void normalize() { a[N_WORDS - 1] &= UPPER_MASK; }

	void assign_zero() { avx_memzero(a, sizeof(a)); }
	void assign_one()  { assign_zero(); a[0] = 1; }
	void assign(const void* buffer) { avx_memcpy(a, buffer, N_BYTES); normalize(); }
	void assign(int x) { *this = x; }

	mp_limb_t get_limb(int i) const { return a[i]; }
	bool get_bit(int i) const;

	const void* get_ptr() const { return a; }

	void negate() { 
		throw not_implemented();
	}
	
	Z2<K> operator+(const Z2<K>& other) const;
	Z2<K> operator-(const Z2<K>& other) const;

	template <int L>
	Z2<K+L> operator*(const Z2<L>& other) const;

	Z2<K> operator*(bool other) const { return other ? *this : Z2<K>(); }

	Z2<K> operator/(const Z2& other) const { (void) other; throw not_implemented(); }

	Z2<K>& operator+=(const Z2<K>& other);
	Z2<K>& operator-=(const Z2<K>& other);

	Z2<K> operator<<(int i) const;
	Z2<K> operator>>(int i) const;

	bool operator==(const Z2<K>& other) const;
	bool operator!=(const Z2<K>& other) const { return not (*this == other); }

	void add(const Z2<K>& a, const Z2<K>& b) { *this = a + b; }
	void add(const Z2<K>& a) { *this += a; }
	void sub(const Z2<K>& a, const Z2<K>& b) { *this = a - b; }

	template <int M, int L>
	void mul(const Z2<M>& a, const Z2<L>& b) { *this = Z2<K>::Mul(a, b); }
	template <int L>
	void mul(const Integer& a, const Z2<L>& b) { *this = Z2<K>::Mul(Z2<64>(a), b); }

	template <int t>
	void add(octetStream& os) { add(os.consume(size())); }

	Z2& invert();

	Z2 sqrRoot();

	bool is_zero() { return *this == Z2<K>(); }

	void randomize(PRNG& G);
	void almost_randomize(PRNG& G) { randomize(G); }

	void pack(octetStream& o) const;
	void unpack(octetStream& o);

	void input(istream& s, bool human=true);
	void output(ostream& s, bool human=true) const;

	template <int J>
	friend ostream& operator<<(ostream& o, const Z2<J>& x);
};

template<int K>
inline Z2<K> Z2<K>::operator+(const Z2<K>& other) const
{
    Z2<K> res;
    mpn_add_fixed_n<N_WORDS>(res.a, a, other.a);
    res.a[N_WORDS - 1] &= UPPER_MASK;
    return res;
}

template<int K>
Z2<K> Z2<K>::operator-(const Z2<K>& other) const
{
	Z2<K> res;
	mpn_sub_fixed_n<N_WORDS>(res.a, a, other.a);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
inline Z2<K>& Z2<K>::operator+=(const Z2<K>& other)
{
	mpn_add_fixed_n<N_WORDS>(a, other.a, a);
	a[N_WORDS - 1] &= UPPER_MASK;
	return *this;
}

template <int K>
Z2<K>& Z2<K>::operator-=(const Z2<K>& other)
{
	*this = *this - other;
	return *this;
}

template <int K>
template <int L, int M>
inline Z2<K> Z2<K>::Mul(const Z2<L>& x, const Z2<M>& y)
{
	Z2<K> res;
	mpn_mul_fixed_<N_WORDS, Z2<L>::N_WORDS, Z2<M>::N_WORDS>(res.a, x.a, y.a);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
template <int L>
inline Z2<K+L> Z2<K>::operator*(const Z2<L>& other) const
{
	return Z2<K+L>::Mul(*this, other);
}

template <int K>
Z2<K> Z2<K>::operator<<(int i) const
{
	Z2<K> res;
	int n_limb_shift = i / N_LIMB_BITS;
	for (int j = n_limb_shift; j < N_WORDS; j++)
		res.a[j] = a[j - n_limb_shift];
	int n_inside_shift = i % N_LIMB_BITS;
	if (n_inside_shift > 0)
		mpn_lshift(res.a, res.a, N_WORDS, n_inside_shift);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
Z2<K> Z2<K>::operator>>(int i) const
{
	Z2<K> res;
	int n_byte_shift = i / 8;
	memcpy(res.a, (char*)a + n_byte_shift, N_BYTES - n_byte_shift);
	int n_inside_shift = i % 8;
	if (n_inside_shift > 0)
		mpn_rshift(res.a, res.a, N_WORDS, n_inside_shift);
	return res;
}

template<int K>
void Z2<K>::randomize(PRNG& G)
{
	G.get_octets<N_BYTES>((octet*)a);
	normalize();
}

template<int K>
void Z2<K>::pack(octetStream& o) const
{
	o.append((octet*)a, N_BYTES);
}

template<int K>
void Z2<K>::unpack(octetStream& o)
{
	o.consume((octet*)a, N_BYTES);
}

#endif /* MATH_Z2K_H_ */
