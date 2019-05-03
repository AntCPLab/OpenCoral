/*
 * MMO.cpp
 *
 *
 */

#include "MMO.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/bigint.h"
#include "Math/Z2k.h"
#include <unistd.h>


void MMO::zeroIV()
{
    if (N_KEYS > (1 << 8))
        throw not_implemented();
    for (int i = 0; i < N_KEYS; i++)
    {
        octet key[AES_BLK_SIZE];
        memset(key, 0, AES_BLK_SIZE * sizeof(octet));
        key[i] = i;
        setIV(i, key);
    }
}


void MMO::setIV(int i, octet key[AES_BLK_SIZE])
{
    aes_schedule(IV[i],key);
}


template<int N>
void MMO::encrypt_and_xor(void* output, const void* input, const octet* key,
        const int* indices)
{
    __m128i in[N], out[N];
    for (int i = 0; i < N; i++)
        in[i] = _mm_loadu_si128(((__m128i*)input) + indices[i]);
    encrypt_and_xor<N>(out, in, key);
    for (int i = 0; i < N; i++)
        _mm_storeu_si128(((__m128i*)output) + indices[i], out[i]);
}

template <class T, int N>
void MMO::hashBlocks(void* output, const void* input)
{
    int n_blocks = DIV_CEIL(T::size(), 16);
    if (n_blocks > N_KEYS)
        throw runtime_error("not enough MMO keys");
    __m128i tmp[N];
    int block_size = sizeof(tmp[0]);
    for (int i = 0; i < n_blocks; i++)
    {
        encrypt_and_xor<N>(tmp, input, IV[i]);
        for (int j = 0; j < N; j++)
            memcpy((char*)output + j * sizeof(T) + i * block_size, &tmp[j],
                    min(T::size() - i * block_size, block_size));
    }
    for (int j = 0; j < N; j++)
        ((T*)output + j)->normalize();
}

template <>
void MMO::hashBlocks<gfp1, 1>(void* output, const void* input)
{
    if (gfp1::get_ZpD().get_t() != 2)
        throw not_implemented();
    encrypt_and_xor<1>(output, input, IV[0]);
    while (mpn_cmp((mp_limb_t*)output, gfp1::get_ZpD().get_prA(), gfp1::t()) >= 0)
        encrypt_and_xor<1>(output, output, IV[0]);
}

template <>
void MMO::hashBlockWise<gf2n,128>(octet* output, octet* input)
{
    for (int i = 0; i < 16; i++)
        encrypt_and_xor<8>(&((__m128i*)output)[i*8], &((__m128i*)input)[i*8], IV[0]);
}

template <>
void MMO::hashBlocks<gfp1, 8>(void* output, const void* input)
{
    if (gfp1::get_ZpD().get_t() != 2)
        throw not_implemented();
    __m128i* in = (__m128i*)input;
    __m128i* out = (__m128i*)output;
    encrypt_and_xor<8>(out, in, IV[0]);
    int left = 8;
    int indices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    while (left)
    {
        int now_left = 0;
        for (int j = 0; j < left; j++)
            if (mpn_cmp((mp_limb_t*)&out[indices[j]], gfp1::get_ZpD().get_prA(), gfp1::t()) >= 0)
            {
                indices[now_left] = indices[j];
                now_left++;
            }
        left = now_left;

        // and now my favorite hack
        switch (left) {
        case 8:
            ecb_aes_128_encrypt<8>(out, out, IV[0], indices);
            break;
        case 7:
            ecb_aes_128_encrypt<7>(out, out, IV[0], indices);
            break;
        case 6:
            ecb_aes_128_encrypt<6>(out, out, IV[0], indices);
            break;
        case 5:
            ecb_aes_128_encrypt<5>(out, out, IV[0], indices);
            break;
        case 4:
            ecb_aes_128_encrypt<4>(out, out, IV[0], indices);
            break;
        case 3:
            ecb_aes_128_encrypt<3>(out, out, IV[0], indices);
            break;
        case 2:
            ecb_aes_128_encrypt<2>(out, out, IV[0], indices);
            break;
        case 1:
            ecb_aes_128_encrypt<1>(out, out, IV[0], indices);
            break;
        default:
            break;
        }
    }
}

template <>
void MMO::hashBlockWise<gfp1,128>(octet* output, octet* input)
{
    for (int i = 0; i < 16; i++)
    {
        __m128i* in = &((__m128i*)input)[i*8];
        __m128i* out = &((__m128i*)output)[i*8];
        hashBlocks<gfp1,8>(out, in);
    }
}

#define ZZ(F,N) \
    template void MMO::hashBlocks<F,N>(void*, const void*);
#define Z(F) ZZ(F,1) ZZ(F,2) ZZ(F,8)
Z(gf2n_long) Z(Z2<64>) Z(Z2<112>) Z(Z2<128>) Z(Z2<160>) Z(Z2<114>) Z(Z2<130>)
Z(Z2<72>)
Z(SignedZ2<64>) Z(SignedZ2<72>)
Z(gf2n_short)
