// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * prf.h
 *
 */

#ifndef PROTOCOL_INC_PRF_H_
#define PROTOCOL_INC_PRF_H_

#include "Key.h"
#include "aes.h"

#include "Tools/aes.h"

void PRF_single(const Key& key, char* input, char* output);

inline Key PRF_single(const Key& key, const Key& input)
{
	Key output;
	PRF_single(key, (char*)&input, (char*)&output);
	return output;
}

inline void PRF_chunk(const Key& key, char* input, char* output, int number)
{
	__m128i* in = (__m128i*)input;
	__m128i* out = (__m128i*)output;
	AES_KEY aes_key;
	AES_128_Key_Expansion((unsigned char*)&key.r, &aes_key);
	switch (number)
	{
	case 2:
		ecb_aes_128_encrypt<2>(out, in, (octet*)aes_key.rd_key);
		break;
	case 3:
		ecb_aes_128_encrypt<3>(out, in, (octet*)aes_key.rd_key);
		break;
	default:
		throw not_implemented();
	}
}

#endif /* PROTOCOL_INC_PRF_H_ */
