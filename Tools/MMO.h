/*
 * MMO.h
 *
 */

#ifndef TOOLS_MMO_H_
#define TOOLS_MMO_H_

#include "Tools/aes.h"
#include "BMR/Key.h"

// Matyas-Meyer-Oseas hashing
class MMO
{
    octet IV[176]  __attribute__((aligned (16)));

    template<int N>
    static void encrypt_and_xor(void* output, const void* input,
            const octet* key);
    template<int N>
    static void encrypt_and_xor(void* output, const void* input,
            const octet* key, const int* indices);

public:
    MMO() { zeroIV(); }
    void zeroIV();
    void setIV(octet key[AES_BLK_SIZE]);
    template <class T>
    void hashOneBlock(octet* output, octet* input);
    template <class T, int N>
    void hashBlockWise(octet* output, octet* input);
    template <class T>
    void outputOneBlock(octet* output);
    Key hash(const Key& input);
    template <int N>
    void hash(Key* output, const Key* input);
};

template<int N>
inline void MMO::encrypt_and_xor(void* output, const void* input, const octet* key)
{
    __m128i in[N], out[N];
    avx_memcpy(in, input, sizeof(in));
    ecb_aes_128_encrypt<N>(out, in, key);
    for (int i = 0; i < N; i++)
        out[i] = _mm_xor_si128(out[i], in[i]);
    avx_memcpy(output, out, sizeof(out));
}

inline Key MMO::hash(const Key& input)
{
    Key res;
    encrypt_and_xor<1>(&res.r, &input.r, IV);
    return res;
}

template <int N>
inline void MMO::hash(Key* output, const Key* input)
{
    encrypt_and_xor<N>(output, input, IV);
}

#endif /* TOOLS_MMO_H_ */
