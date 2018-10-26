/*
 * Rep3Share.h
 *
 */

#ifndef MATH_REP3SHARE_H_
#define MATH_REP3SHARE_H_

#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Processor/Replicated.h"

class Rep3Share: public FixedVec<Integer, 2>
{
public:
    typedef Integer clear;

    typedef Replicated<Rep3Share> Protocol;
    typedef ReplicatedMC<Rep3Share> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<Rep3Share> Input;
    typedef ReplicatedPrivateOutput<Rep3Share> PrivateOutput;

    static char type_char()
    {
        return clear::type_char();
    }

    Rep3Share()
    {
    }
    Rep3Share(const FixedVec<Integer, 2>& other)
    {
        FixedVec<Integer, 2>::operator=(other);
    }

    Rep3Share(Integer value, int my_num)
    {
        Replicated<Rep3Share>::assign(*this, value, my_num);
    }

    // Share<T> compatibility
    void assign(clear other, int my_num, const Integer& alphai)
    {
        (void)alphai;
        *this = Rep3Share(other, my_num);
    }
    void assign(const char* buffer)
    {
        FixedVec<Integer, 2>::assign(buffer);
    }

    void add(const Rep3Share& x, const Rep3Share& y)
    {
        *this = x + y;
    }
    void sub(const Rep3Share& x, const Rep3Share& y)
    {
        *this = x - y;
    }

    void add(const Rep3Share& S, const clear aa, int my_num,
            const Integer& alphai)
    {
        (void)alphai;
        *this = S + Rep3Share(aa, my_num);
    }
    void sub(const Rep3Share& S, const clear& aa, int my_num,
            const Integer& alphai)
    {
        (void)alphai;
        *this = S - Rep3Share(aa, my_num);
    }
    void sub(const clear& aa, const Rep3Share& S, int my_num,
            const Integer& alphai)
    {
        (void)alphai;
        *this = Rep3Share(aa, my_num) - S;
    }

    void pack(octetStream& os, bool full = true) const
    {
        (void)full;
        FixedVec<Integer, 2>::pack(os);
    }
    void unpack(octetStream& os, bool full = true)
    {
        (void)full;
        FixedVec<Integer, 2>::unpack(os);
    }
};

#endif /* MATH_REP3SHARE_H_ */
