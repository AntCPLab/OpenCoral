/*
 * FixedVec.h
 *
 */

#ifndef MATH_FIXEDVEC_H_
#define MATH_FIXEDVEC_H_

#include <string>
using namespace std;

#include "Tools/octetStream.h"
#include "Tools/random.h"
#include "field_types.h"

template<class T> class ReplicatedMC;
template<class T> class ReplicatedInput;
template<class T> class ReplicatedPrivateOutput;
template<class T> class Replicated;

template <class T, int L>
class FixedVec
{
    T v[L];

public:
    typedef T value_type;
    static const int length = L;

    typedef ReplicatedMC<FixedVec<T, L> > MC;
    typedef ReplicatedInput<FixedVec<T, L> > Inp;
    typedef ReplicatedPrivateOutput<FixedVec<T, L> > PO;
    typedef Replicated<FixedVec<T, L> > Protocol;

    static int size()
    {
        return L * T::size();
    }
    static string type_string()
    {
        return T::type_string() + "^" + to_string(L);
    }
    static DataFieldType field_type()
    {
        return DATA_MODP;
    }

    FixedVec<T, L>(const T& other = 0)
    {
        for (auto& x : v)
            x = other;
    }

    FixedVec<T, L>(long other) :
            FixedVec<T, L>(T(other))
    {
    }

    T& operator[](int i)
    {
        return v[i];
    }
    const T& operator[](int i) const
    {
        return v[i];
    }

    void assign(const T& other)
    {
        for (auto& x : v)
            x = other;
    }
    void assign(const char* buffer)
    {
        for (auto& x : v)
            x.assign(buffer);
    }

    void assign_zero()
    {
        for (auto& x : v)
            x = 0;
    }
    void assign_one()
    {
        assign(1);
    }

    void add(const FixedVec<T, L>& x, const FixedVec<T, L>& y)
    {
        for (int i = 0; i < L; i++)
            v[i] = x.v[i] + y.v[i];
    }
    void sub(const FixedVec<T, L>& x, const FixedVec<T, L>& y)
    {
        for (int i = 0; i < L; i++)
            v[i] = x.v[i] - y.v[i];
    }

    void mul(const FixedVec<T, L>& x, const FixedVec<T, L>& y)
    {
        for (int i = 0; i < L; i++)
            v[i] = x.v[i] * y.v[i];
    }

    void add(const FixedVec<T, L>& x)
    {
        add(*this, x);
    }

    void negate()
    {
        for (auto& x : v)
            x = -x;
    }

    bool equal(const FixedVec<T, L>& x) const
    {
        for (int i = 0; i < L; i++)
            if (v[i] != x[i])
                return false;
        return true;
    }

    bool is_zero()
    {
        throw not_implemented();
    }
    bool is_one()
    {
        throw not_implemented();
    }

    FixedVec<T, L>operator^(const FixedVec<T, L>& other) const
    {
        FixedVec<T, L> res;
        for (int i = 0; i < L; i++)
            res[i] = v[i] ^ other[i];
        return res;
    }

    FixedVec<T, L>operator&(const FixedVec<T, L>& other) const
    {
        FixedVec<T, L> res;
        for (int i = 0; i < L; i++)
            res[i] = v[i] & other[i];
        return res;
    }

    FixedVec<T, L>& operator^=(const FixedVec<T, L>& other)
    {
        for (int i = 0; i < L; i++)
            v[i] ^= other[i];
        return *this;
    }

    FixedVec<T, L>& operator&=(const FixedVec<T, L>& other)
    {
        for (int i = 0; i < L; i++)
            v[i] &= other[i];
        return *this;
    }

    FixedVec<T, L> operator<<(int i) const
    {
        FixedVec<T ,L> res;
        for (int j = 0; j < L; j++)
            res[j] = v[j] << i;
        return res;
    }

    FixedVec<T, L> operator>>(int i) const
    {
        FixedVec<T ,L> res;
        for (int j = 0; j < L; j++)
            res[j] = v[j] >> i;
        return res;
    }

    T sum() const
    {
        T res = 0;
        for (auto& x : v)
            res += x;
        return res;
    }

    FixedVec<T, L> extend_bit() const
    {
        FixedVec<T, L> res;
        for (int i = 0; i < L; i++)
            res[i] = v[i].extend_bit();
        return res;
    }

    void randomize(PRNG& G)
    {
        for (auto& x : v)
            x.randomize(G);
    }
    void randomize_to_sum(const T& sum, PRNG& G)
    {
        T s = 0;
        for (int i = 1; i < L; i++)
        {
            v[i].randomize(G);
            s += v[i];
        }
        v[0] = sum - s;
    }

    void output(ostream& s, bool human) const
    {
        for (auto& x : v)
            x.output(s, human);
    }
    void input(istream& s, bool human)
    {
        for (auto& x : v)
            x.input(s, human);
    }

    void pack(octetStream& os) const
    {
        for (auto& x : v)
            x.pack(os);
    }
    void unpack(octetStream& os)
    {
        for (auto& x : v)
            x.unpack(os);
    }
};

template <class T, int L>
ostream& operator<<(ostream& os, const FixedVec<T, L>& v)
{
    for (int i = 0; i < L; i++)
    {
        os << v[i];
        if (i < L - 1)
            os << ",";
    }
    return os;
}

#endif /* MATH_FIXEDVEC_H_ */
