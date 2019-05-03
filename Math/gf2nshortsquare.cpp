/*
 * gf2nSquare.cpp
 *
 */

#include "gf2nshortsquare.h"


template<>
gf2n_short_square& gf2n_short_square::sub<gf2n_short>(const gf2n_short_square& other)
{
    for (int i = 0; i < gf2n_short::degree(); i++)
        rows[i] -= other.rows[i];
    return *this;
}

template<>
gf2n_short_square& gf2n_short_square::sub<gf2n_short>(const void* other)
{
    gf2n_short value = *(word*)other;
    for (int i = 0; i < gf2n_short::degree(); i++)
        rows[i] -= value;
    return *this;
}

template<>
void gf2n_short_square::conditional_add<gf2n_short>(BitVector& conditions,
        gf2n_short_square& other, int offset)
{
    for (int i = 0; i < gf2n_short::degree(); i++)
        if (conditions.get_bit(N_ROWS * offset + i))
            rows[i] += other.rows[i];
}

void gf2n_short_square::to(gf2n_short& result)
{
    int128 sum;
    for (int i = 0; i < gf2n_short::degree(); i++)
        sum ^= int128(rows[i].get()) << i;
    result = sum;
}

void gf2n_short_square::pack(octetStream& os) const
{
    for (auto& x : rows)
        x.pack(os);
}

void gf2n_short_square::unpack(octetStream& os)
{
    for (auto& x : rows)
        x.unpack(os);
}
