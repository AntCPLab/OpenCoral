/*
 * Rep3Share.h
 *
 */

#ifndef PROTOCOLS_REP3SHARE_H_
#define PROTOCOLS_REP3SHARE_H_

#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Protocols/Replicated.h"
#include "GC/ShareSecret.h"
#include "ShareInterface.h"

template<class T> class ReplicatedPrep;
template<class T> class PrivateOutput;

template<class T>
class Rep3Share : public FixedVec<T, 2>, public ShareInterface
{
public:
    typedef T clear;
    typedef T open_type;
    typedef T mac_type;
    typedef T mac_key_type;

    typedef Replicated<Rep3Share> Protocol;
    typedef ReplicatedMC<Rep3Share> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<Rep3Share> Input;
    typedef ::PrivateOutput<Rep3Share> PrivateOutput;
    typedef ReplicatedPrep<Rep3Share> LivePrep;
    typedef Rep3Share Honest;

    typedef GC::SemiHonestRepSecret bit_type;

    const static bool needs_ot = false;
    const static bool dishonest_majority = false;
    const static bool expensive = false;

    static string type_short()
    {
        return "R" + string(1, clear::type_char());
    }
    static string type_string()
    {
        return "replicated " + T::type_string();
    }

    static int threshold(int)
    {
        return 1;
    }

    static Rep3Share constant(T value, int my_num, const T& alphai = {})
    {
        return Rep3Share(value, my_num, alphai);
    }

    Rep3Share()
    {
    }
    template<class U>
    Rep3Share(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }

    Rep3Share(T value, int my_num, const T& alphai = {})
    {
        (void) alphai;
        Replicated<Rep3Share>::assign(*this, value, my_num);
    }

    // Share<T> compatibility
    void assign(clear other, int my_num, const T& alphai)
    {
        (void)alphai;
        *this = Rep3Share(other, my_num);
    }
    void assign(const char* buffer)
    {
        FixedVec<T, 2>::assign(buffer);
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
            const T& alphai)
    {
        (void)alphai;
        *this = S + Rep3Share(aa, my_num);
    }
    void sub(const Rep3Share& S, const clear& aa, int my_num,
            const T& alphai)
    {
        (void)alphai;
        *this = S - Rep3Share(aa, my_num);
    }
    void sub(const clear& aa, const Rep3Share& S, int my_num,
            const T& alphai)
    {
        (void)alphai;
        *this = Rep3Share(aa, my_num) - S;
    }

    clear local_mul(const Rep3Share& other) const
    {
        T a, b;
        a.mul((*this)[0], other.sum());
        b.mul((*this)[1], other[0]);
        return a + b;
    }

    void mul_by_bit(const Rep3Share& x, const T& y)
    {
        (void) x, (void) y;
        throw runtime_error("multiplication by bit not implemented");
    }

    void pack(octetStream& os, bool full = true) const
    {
        if (full)
            FixedVec<T, 2>::pack(os);
        else
            (*this)[0].pack(os);
    }
    void unpack(octetStream& os, bool full = true)
    {
        assert(full);
        FixedVec<T, 2>::unpack(os);
    }
};

#endif /* PROTOCOLS_REP3SHARE_H_ */
