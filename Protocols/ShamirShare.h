/*
 * ShamirShare.h
 *
 */

#ifndef PROTOCOLS_SHAMIRSHARE_H_
#define PROTOCOLS_SHAMIRSHARE_H_

#include "Protocols/Shamir.h"
#include "Protocols/ShamirInput.h"
#include "Machines/ShamirMachine.h"

template<class T> class ReplicatedPrep;

template<class T>
class ShamirShare : public T
{
public:
    typedef T clear;
    typedef T open_type;
    typedef T mac_key_type;

    typedef Shamir<T> Protocol;
    typedef ShamirMC<ShamirShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ShamirInput<ShamirShare> Input;
    typedef ::PrivateOutput<ShamirShare> PrivateOutput;
    typedef ReplicatedPrep<ShamirShare> LivePrep;
    typedef ShamirShare Honest;

    const static bool needs_ot = false;
    const static bool dishonest_majority = false;

    static string type_short()
    {
        return "S" + string(1, clear::type_char());
    }
    static string type_string()
    {
        return "Shamir " + T::type_string();
    }

    static int threshold(int)
    {
        return ShamirMachine::s().threshold;
    }

    static ShamirShare constant(T value, int my_num, const T& alphai = {})
    {
        return ShamirShare(value, my_num, alphai);
    }

    ShamirShare()
    {
    }
    template<class U>
    ShamirShare(const U& other)
    {
        T::operator=(other);
    }
    template<class U>
    ShamirShare(const U& other, int my_num, T alphai = {}) : ShamirShare(other)
    {
        (void) my_num, (void) alphai;
    }

    // Share<T> compatibility
    void assign(clear other, int my_num, const T& alphai)
    {
        (void)alphai, (void)my_num;
        *this = other;
    }
    void assign(const char* buffer)
    {
        T::assign(buffer);
    }

    void add(const ShamirShare& x, const ShamirShare& y)
    {
        *this = x + y;
    }
    void sub(const ShamirShare& x, const ShamirShare& y)
    {
        *this = x - y;
    }

    void add(const ShamirShare& S, const clear aa, int my_num,
            const T& alphai)
    {
        (void) my_num, (void) alphai;
        *this = S + aa;
    }
    void sub(const ShamirShare& S, const clear& aa, int my_num,
            const T& alphai)
    {
        (void) my_num, (void) alphai;
        *this = S - aa;
    }
    void sub(const clear& aa, const ShamirShare& S, int my_num,
            const T& alphai)
    {
        (void) my_num, (void) alphai;
        *this = aa - S;
    }

    ShamirShare operator<<(int i)
    {
        return *this * (T(1) << i);
    }
    ShamirShare& operator<<=(int i)
    {
        *this = *this << i;
        return *this;
    }

    void force_to_bit()
    {
        throw not_implemented();
    }

    void pack(octetStream& os, bool full = true) const
    {
        (void)full;
        T::pack(os);
    }
    void unpack(octetStream& os, bool full = true)
    {
        (void)full;
        T::unpack(os);
    }
};

#endif /* PROTOCOLS_SHAMIRSHARE_H_ */
