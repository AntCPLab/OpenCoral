/*
 * Key.h
 *
 */

#ifndef COMMON_INC_KEY_H_
#define COMMON_INC_KEY_H_

#include <iostream>
#include <emmintrin.h>
#include <smmintrin.h>
#include <string.h>

#include "Tools/FlexBuffer.h"
#include "Math/gf2nlong.h"

using namespace std;

#ifndef __PRIME_FIELD__

class Key {
public:
	__m128i r;

	Key() {}
	Key(long long a) : r(_mm_cvtsi64_si128(a)) {}
	Key(long long a, long long b) : r(_mm_set_epi64x(a, b)) {}
	Key(__m128i r) : r(r) {}
//	Key(const Key& other) {r= other.r;}

//	Key& operator=(const Key& other);
	bool operator==(const Key& other);
	bool operator!=(const Key& other) { return !(*this == other); }

	Key& operator-=(const Key& other);
	Key& operator+=(const Key& other);

	Key operator^(const Key& other) const { return r ^ other.r; }
	Key operator^=(const Key& other) { r ^= other.r; return *this; }

	void serialize(SendBuffer& output) const { output.serialize(r); }
	void serialize_no_allocate(SendBuffer& output) const { output.serialize_no_allocate(r); }

	bool get_signal() const { return _mm_cvtsi128_si64(r) & 1; }
	void set_signal(bool signal);

	Key doubling(int i) const;

	template <class T>
	T get() const;
};

ostream& operator<<(ostream& o, const Key& key);
ostream& operator<<(ostream& o, const __m128i& x);


inline bool Key::operator==(const Key& other) {
    return int128(r) == other.r;
}

inline Key& Key::operator-=(const Key& other) {
    r ^= other.r;
    return *this;
}

inline Key& Key::operator+=(const Key& other) {
    r ^= other.r;
    return *this;
}

template <>
inline unsigned long Key::get() const
{
	return _mm_cvtsi128_si64(r);
}

template <>
inline __m128i Key::get() const
{
	return r;
}

inline void Key::set_signal(bool signal)
{
	r &= ~_mm_cvtsi64_si128(1);
	r ^= _mm_cvtsi64_si128(signal);
}

inline Key Key::doubling(int i) const
{
#ifdef __AVX2__
	return _mm_sllv_epi64(r, _mm_set_epi64x(i, i));
#else
	uint64_t halfs[2];
	halfs[1] = _mm_cvtsi128_si64(_mm_unpackhi_epi64(r, r)) << i;
	halfs[0] = _mm_cvtsi128_si64(r) << i;
	return _mm_loadu_si128((__m128i*)halfs);
#endif
}


#else //__PRIME_FIELD__ is defined

const __uint128_t MODULUS= 0xffffffffffffffffffffffffffffff61;
#define MODP_STR "ffffffffffffffffffffffffffffff61"

#ifdef __PURE_SHE__
#include "mpir.h"
extern mpz_t key_modulo;
void init_modulos();
void init_temp_mpz_t(mpz_t& temp);
#endif

class Key {
public:
	__uint128_t r;

	Key() {r=0;}
	Key(__uint128_t a) {r=a;}
	Key(const Key& other) {r= other.r;}

	Key& operator=(const Key& other){r= other.r; return *this;}
	bool operator==(const Key& other) {return r == other.r;}
	Key& operator-=(const Key& other) {r-=other.r; r%=MODULUS; return *this;}
	Key& operator+=(const Key& other) {r+=other.r; r%=MODULUS; return *this;}

	Key& operator=(const __uint128_t& other){r= other; return *this;}
	bool operator==(const __uint128_t& other) {return r == other;}
	Key& operator-=(const __uint128_t& other) {r-=r; r%=MODULUS; return *this;}
	Key& operator+=(const __uint128_t& other) {r+=other; r%=MODULUS; return *this;}
#if __PURE_SHE__
	inline __uint128_t sqr(mpz_t& temp) {
		temp->_mp_size = 2;
		*((__int128*)temp->_mp_d) = r;
		mpz_mul(temp, temp, temp);
		mpz_tdiv_r(temp, temp, key_modulo);
		__uint128_t ret = *((__int128*)temp->_mp_d);
		return ret;
	}
	inline Key& sqr_in_place(mpz_t& temp) {
		temp->_mp_size = 2;
		*((__int128*)temp->_mp_d) = r;
		mpz_mul(temp, temp, temp);
		mpz_tdiv_r(temp, temp, key_modulo);
		r = *((__int128*)temp->_mp_d);
		return *this;
	}
#else
	inline Key& sqr() {r=r*r; r%=MODULUS; return *this;}
#endif
	inline void adjust() {r=r%MODULUS;}
};

ostream& operator<<(ostream& o, const Key& key);
ostream& operator<<(ostream& o, const __uint128_t& x);


#endif

#endif /* COMMON_INC_KEY_H_ */
