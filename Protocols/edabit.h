/*
 * edabit.h
 *
 */

#ifndef PROTOCOLS_EDABIT_H_
#define PROTOCOLS_EDABIT_H_

#include "Tools/FixedVector.h"

template<class T>
using edabit = pair<T, FixedVector<typename T::bit_type::part_type::small_type, T::clear::MAX_EDABITS + 5>>;

template<class T>
class edabitvec
{
    typedef FixedVector<typename T::bit_type::part_type, T::clear::MAX_EDABITS + 5> b_type;

    FixedVector<T, T::bit_type::part_type::default_length> a;
    b_type b;

public:
    edabitvec()
    {
    }

    edabitvec(const edabit<T>& other) :
            edabitvec(other.first, other.second)
    {
    }

    edabitvec(const T& aa, const typename edabit<T>::second_type& bb)
    {
        a.push_back(aa);
        for (auto& x : bb)
            b.push_back(x);
    }

    edabitvec(const vector<typename T::bit_type::part_type>& other)
    {
        b = other;
    }

    void push_a(const T& x)
    {
        a.push_back(x);
    }

    bool empty()
    {
        return a.empty();
    }

    bool full()
    {
        return a.full();
    }

    size_t size()
    {
        return a.size();
    }

    T get_a(int i)
    {
        return a[i];
    }

    typename T::bit_type get_b(int i)
    {
        return b[i];
    }

    edabit<T> next()
    {
        edabit<T> res;
        res.first = a.pop();
        if (T::bit_type::part_type::default_length > 1)
            for (auto& x : b)
                res.second.push_back(x.get_bit(a.size()));
        else
            for (auto& x : b)
                res.second.push_back(x);
        return res;
    }

    void push_back(const edabit<T>& x)
    {
        for (size_t i = 0; i < x.second.size(); i++)
        {
            b[i] ^= typename T::bit_type::part_type(x.second[i]) << a.size();
        }
        a.push_back(x.first);
    }
};

#endif /* PROTOCOLS_EDABIT_H_ */
