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
#include "Math/BitVec.h"
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

template <int N, int N_BYTES>
void MMO::hashBlocks(void* output, const void* input, size_t alloc_size)
{
    size_t used_size = N_BYTES;
    int n_blocks = DIV_CEIL(used_size, 16);
    if (n_blocks > N_KEYS)
        throw runtime_error("not enough MMO keys");
    __m128i tmp[N];
    size_t block_size = sizeof(tmp[0]);
    for (int i = 0; i < n_blocks; i++)
    {
        encrypt_and_xor<N>(tmp, input, IV[i]);
        for (int j = 0; j < N; j++)
            memcpy((char*)output + j * alloc_size + i * block_size, &tmp[j],
                    min(used_size - i * block_size, block_size));
    }
}

template <class T, int N>
void MMO::hashBlocks(void* output, const void* input)
{
    hashBlocks<N, T::N_BYTES>(output, input, sizeof(T));
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

template <int X, int L>
void MMO::hashEightGfp(void* output, const void* input)
{
    if (gfp_<X, L>::get_ZpD().get_t() < 2)
        throw not_implemented();
    gfp_<X, L>* out = (gfp_<X, L>*)output;
    hashBlocks<8, gfp_<X, L>::N_BYTES>(output, input, sizeof(gfp_<X, L>));
    for (int i = 0; i < 8; i++)
        out[i].zero_overhang();
    int left = 8;
    int indices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    while (left)
    {
        int now_left = 0;
        for (int j = 0; j < left; j++)
            if (mpn_cmp((mp_limb_t*) out[indices[j]].get_ptr(),
                    gfp_<X, L>::get_ZpD().get_prA(), gfp_<X, L>::t()) >= 0)
            {
                indices[now_left] = indices[j];
                now_left++;
            }
        left = now_left;

        int block_size = sizeof(__m128i);
        int n_blocks = DIV_CEIL(gfp_<X, L>::size(), block_size);
        for (int i = 0; i < n_blocks; i++)
            for (int j = 0; j < left; j++)
            {
                __m128i* addr = (__m128i*) out[indices[j]].get_ptr() + i;
                __m128i* in = (__m128i*) out[indices[j]].get_ptr();
                auto tmp = aes_128_encrypt(_mm_loadu_si128(in), IV[i]);
                memcpy(addr, &tmp, min(block_size, gfp_<X, L>::size() - i * block_size));
                out[indices[j]].zero_overhang();
            }
    }
}

template <>
void MMO::hashBlocks<gfp1, 8>(void* output, const void* input)
{
    hashEightGfp<1, GFP_MOD_SZ>(output, input);
}

template <>
void MMO::hashBlocks<gfp3, 8>(void* output, const void* input)
{
    hashEightGfp<3, 4>(output, input);
}

#define ZZ(F,N) \
    template void MMO::hashBlocks<F,N>(void*, const void*);
#define Z(F) ZZ(F,1) ZZ(F,2) ZZ(F,8)
Z(gf2n_long) Z(Z2<64>) Z(Z2<112>) Z(Z2<128>) Z(Z2<160>) Z(Z2<114>) Z(Z2<130>)
Z(Z2<72>)
Z(SignedZ2<64>) Z(SignedZ2<72>)
Z(gf2n_short)
Z(BitVec)
Z(Z2<41>)
Z(Z2<120>) Z(Z2<122>) Z(Z2<136>) Z(Z2<138>)
