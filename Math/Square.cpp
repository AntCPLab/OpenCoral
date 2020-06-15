/*
 * Square.cpp
 *
 */

#include "Square.h"
#include "BitVec.h"
#include "gf2n.h"
#include "gfp.h"

template<>
void Square<gf2n_short>::to(gf2n_short& result)
{
    int128 sum;
    for (int i = 0; i < gf2n_short::degree(); i++)
        sum ^= int128(rows[i].get()) << i;
    result = sum;
}

template<class U>
template<int X, int L>
void Square<U>::to(gfp_<X, L>& result)
{
    mp_limb_t product[2 * L], sum[2 * L], tmp[L][2 * L];
    memset(tmp, 0, sizeof(tmp));
    memset(sum, 0, sizeof(sum));
    for (int i = 0; i < gfp_<X, L>::length(); i++)
    {
        memcpy(&(tmp[i/64][i/64]), &(rows[i]), sizeof(rows[i]));
        if (i % 64 == 0)
            memcpy(product, tmp[i/64], sizeof(product));
        else
            mpn_lshift(product, tmp[i/64], 2 * L, i % 64);
        mpn_add_fixed_n<2 * L>(sum, product, sum);
    }
    mp_limb_t q[2 * L], ans[2 * L];
    mpn_tdiv_qr(q, ans, 0, sum, 2 * L, gfp_<X, L>::get_ZpD().get_prA(), L);
    result.assign((void*) ans);
}

template<>
void Square<gfp1>::to(gfp1& result)
{
    to<1, GFP_MOD_SZ>(result);
}

template<>
void Square<gfp3>::to(gfp3& result)
{
    to<3, 4>(result);
}

template<>
void Square<BitVec>::to(BitVec& result)
{
    result = 0;
    for (int i = 0; i < N_ROWS; i++)
        result ^= ((rows[i] >> i) & 1) << i;
}
