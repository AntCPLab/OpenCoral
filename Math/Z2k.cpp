/*
 * Z2k.cpp
 *
 */

#include <Math/Z2k.h>

template<int K>
Z2<K>::Z2(const bigint& x) : Z2()
{
	auto mp = x.get_mpz_t();
	memcpy(a, mp->_mp_d, sizeof(mp_limb_t) * min(N_WORDS, abs(mp->_mp_size)));
	if (mp->_mp_size < 0)
		*this = Z2<K>() - *this;
}

template<int K>
bool Z2<K>::get_bit(int i) const
{
	return 1 & (a[i / N_LIMB_BITS] >> (i % N_LIMB_BITS));
}

template<int K>
Z2<K> Z2<K>::operator+(const Z2<K>& other) const
{
	Z2<K> res;
	mpn_add(res.a, a, N_WORDS, other.a, N_WORDS);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template<int K>
Z2<K> Z2<K>::operator-(const Z2<K>& other) const
{
	Z2<K> res;
	mpn_sub(res.a, a, N_WORDS, other.a, N_WORDS);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
Z2<K>& Z2<K>::operator+=(const Z2<K>& other)
{
	*this = *this + other;
	return *this;
}

template <int K>
Z2<K> Z2<K>::operator<<(int i) const
{
	Z2<K> res;
	int n_limb_shift = i / N_LIMB_BITS;
	for (int j = n_limb_shift; j < N_WORDS; j++)
		res.a[j] = a[j - n_limb_shift];
	mpn_lshift(res.a, res.a, N_WORDS, i % N_LIMB_BITS);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template<int K>
bool Z2<K>::operator==(const Z2<K>& other) const
{
	return mpn_cmp(a, other.a, N_WORDS) == 0;
}

template<int K>
void Z2<K>::randomize(PRNG& G)
{
	G.get_octets((octet*)a, N_BYTES);
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

template<int K>
void Z2<K>::input(istream& s, bool human)
{
	if (human)
		throw not_implemented();
	s.read((char*)a, N_BYTES);
}

template<int K>
void Z2<K>::output(ostream& s, bool human) const
{
	if (human)
		s << *this;
	else
		s.write((char*)a, N_BYTES);
}

#define NS X(32) X(64) X(96) X(128) X(160) X(192) X(224) X(256) X(320) X(672)
#define X(N) template class Z2<N>;
NS
