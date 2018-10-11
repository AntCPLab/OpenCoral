// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Key.cpp
 *
 */


#include <string.h>
#include "Key.h"

#ifndef __PRIME_FIELD__
ostream& operator<<(ostream& o, const Key& key)
{
	return o << key.r;
}

ostream& operator<<(ostream& o, const __m128i& x) {
	o.fill('0');
	o << hex << noshowbase;
	for (int i = 0; i < 2; i++)
	{
		o.width(16);
		o << ((int64_t*)&x)[1-i];
	}
	o << dec;
	return o;
}

//Key& Key::operator=(const Key& other) {
//	r= other.r;
////	memcpy(&r, &other.r, sizeof(r));
//	return *this;
//}

#else //__PRIME_FIELD__ is defined


#ifdef __PURE_SHE__

mpz_t key_modulo;

void init_modulos()
{
	printf("initiating modulos\n");
	mpz_init2(key_modulo,128);
	mpz_set_str(key_modulo, MODP_STR, 16);
	std::cout << std::hex << "key_modulo: " << key_modulo << std::endl;
	phex(key_modulo->_mp_d, 16);
}

void init_temp_mpz_t(mpz_t& temp) {
	temp->_mp_alloc = 8;
	temp->_mp_size = 2;
	temp->_mp_d = new mp_limb_t[8];
//	*((__int128*)temp->_mp_d) = 1;
}
#endif


ostream& operator<<(ostream& o, const Key& key)
{
	return o << key.r;
}

ostream& operator<<(ostream& o, const __uint128_t& x) {
	o.fill('0');
	o << hex;

	for (int i = 0; i < 2; i++)
	{
		o.width(16);
		o << ((int64_t*)&x)[1-i];
	}
	o << dec;
	return o;
}

#endif

