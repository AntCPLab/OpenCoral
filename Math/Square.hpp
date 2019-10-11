/*
 * gf2nSquare.cpp
 *
 */

#include "Math/Square.h"

template<class U>
Square<U>& Square<U>::sub(const Square<U>& other)
{
    for (int i = 0; i < U::length(); i++)
        rows[i] -= other.rows[i];
    return *this;
}

template<class U>
Square<U>& Square<U>::rsub(const Square<U>& other)
{
    for (int i = 0; i < U::length(); i++)
        rows[i] = other.rows[i] - rows[i];
    return *this;
}

template<class U>
Square<U>& Square<U>::sub(const void* other)
{
    U value;
    value.assign(other);
    for (int i = 0; i < U::length(); i++)
        rows[i] -= value;
    return *this;
}

template<class U>
void Square<U>::conditional_add(BitVector& conditions,
        Square<U>& other, int offset)
{
    for (int i = 0; i < U::length(); i++)
        if (conditions.get_bit(N_ROWS * offset + i))
            rows[i] += other.rows[i];
}

template<class U>
void Square<U>::pack(octetStream& os) const
{
    for (int i = 0; i < U::length(); i++)
        rows[i].pack(os);
}

template<class U>
void Square<U>::unpack(octetStream& os)
{
    for (int i = 0; i < U::length(); i++)
        rows[i].unpack(os);
}
