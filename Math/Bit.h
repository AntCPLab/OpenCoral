/*
 * Bit.h
 *
 */

#ifndef MATH_BIT_H_
#define MATH_BIT_H_

#include "BitVec.h"

class Bit : public BitVec_<bool>
{
    typedef BitVec_<bool> super;

public:
    static int size_in_bits()
    {
        return 1;
    }

    Bit()
    {
    }
    Bit(bool other) :
            super(other)
    {
    }
    Bit(const super::super& other) :
            super(other)
    {
    }

    Bit operator*(const Bit& other) const
    {
        return super::operator*(other);
    }

    template<class T>
    T operator*(const T& other) const
    {
        return other * *this;
    }

    void pack(octetStream& os, int = -1) const
    {
        super::pack(os, 1);
    }
    void unpack(octetStream& os, int = -1)
    {
        super::unpack(os, 1);
    }
};

#endif /* MATH_BIT_H_ */
