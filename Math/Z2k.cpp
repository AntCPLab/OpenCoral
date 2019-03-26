/*
 * Z2k.cpp
 *
 */

#include <Math/Z2k.h>

template<int K>
Z2<K>::Z2(const bigint& x) : Z2()
{
	auto mp = x.get_mpz_t();
	memcpy(a, mp->_mp_d, min((size_t)N_BYTES, sizeof(mp_limb_t) * abs(mp->_mp_size)));
	if (mp->_mp_size < 0)
		*this = Z2<K>() - *this;
}

template<int K>
bool Z2<K>::get_bit(int i) const
{
	return 1 & (a[i / N_LIMB_BITS] >> (i % N_LIMB_BITS));
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
	int n_limb_shift = i / N_LIMB_BITS;
	for (int j = 0; j < N_WORDS - n_limb_shift; j++)
		res.a[j] = a[j + n_limb_shift];
	int n_inside_shift = i % N_LIMB_BITS;
	if (n_inside_shift > 0)
		mpn_rshift(res.a, res.a, N_WORDS, n_inside_shift);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template<int K>
bool Z2<K>::operator==(const Z2<K>& other) const
{
#ifdef DEBUG_MPN
	for (int i = 0; i < N_WORDS; i++)
		cout << "cmp " << hex << a[i] << " " << other.a[i] << endl;
#endif
	return mpn_cmp(a, other.a, N_WORDS) == 0;
}

template<int K>
void Z2<K>::randomize(PRNG& G)
{
	G.get_octets_<N_BYTES>((octet*)a);
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

template <int K>
ostream& operator<<(ostream& o, const Z2<K>& x)
{
	bool printing = false;
	o << "0x" << noshowbase;
	o.width(0);
	for (int i = x.N_WORDS - 1; i >= 0; i--)
		if (x.a[i] or printing or i == 0)
		{
			o << hex << x.a[i];
			printing = true;
			o.width(16);
			o.fill('0');
		}
	o.width(0);
	return o;
}

#define X(N) \
	template class Z2<N>; \
	template ostream& operator<<(ostream& o, const Z2<N>& x);

X(32) X(64) X(96) X(128) X(160) X(192) X(224) X(256) X(288) X(320) X(352) X(384) X(416) X(448) X(512) X(672)
