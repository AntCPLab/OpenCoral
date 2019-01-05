/*
 * ShamirShare.h
 *
 */

#ifndef MATH_SHAMIRSHARE_H_
#define MATH_SHAMIRSHARE_H_

#include "gfp.h"
#include "gf2n.h"
#include "Processor/Shamir.h"
#include "Processor/ShamirInput.h"

template<class T>
class ShamirShare : public T
{
public:
    typedef T clear;

    typedef Shamir<T> Protocol;
    typedef ShamirMC<ShamirShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ShamirInput<ShamirShare> Input;
    typedef ReplicatedPrivateOutput<ShamirShare> PrivateOutput;

    static string type_short()
    {
        return "S" + string(1, clear::type_char());
    }
    static string type_string()
    {
        return "Shamir " + T::type_string();
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
    ShamirShare(const U& other, int my_num) : ShamirShare(other)
    {
        (void) my_num;
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

#endif /* MATH_SHAMIRSHARE_H_ */
