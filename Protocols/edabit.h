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
    typedef FixedVector<T, T::bit_type::part_type::default_length> a_type;

public:
    static const int MAX_SIZE = a_type::MAX_SIZE;

    a_type a;
    b_type b;

    edabitvec()
    {
    }

    edabitvec(const edabit<T>& other) :
            edabitvec(other.first, other.second)
    {
    }

    edabitvec(const T& aa, const typename edabit<T>::second_type& bb)
    {
        if (T::bit_type::is_encoded)
            throw invalid_pack_usage();

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
        if (T::bit_type::is_encoded)
            throw invalid_pack_usage();
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
        if (T::bit_type::is_encoded)
            throw invalid_pack_usage();
        for (size_t i = 0; i < x.second.size(); i++)
        {
            b[i] ^= (typename T::bit_type::part_type(x.second[i])
                    ^ b[i].get_bit(a.size())) << a.size();
        }
        a.push_back(x.first);
    }

    void input(int length, ifstream& s)
    {
        char buffer[MAX_SIZE * T::size()];
        s.read(buffer, MAX_SIZE * T::size());
        for (int i = 0; i < MAX_SIZE; i++)
        {
            T x;
            x.assign(buffer + i * T::size());
            a.push_back(x);
        }
        size_t bsize = T::bit_type::part_type::size();
        char bbuffer[length * bsize];
        s.read(bbuffer, length * bsize);
        for (int i = 0; i < length; i++)
        {
            typename T::bit_type::part_type x;
            x.assign(bbuffer + i * bsize);
            b.push_back(x);
        }
    }

    void output(int length, ofstream& s)
    {
        assert(size() == MAX_SIZE);
        for (auto& x : a)
            x.output(s, false);
        for (int i = 0; i < length; i++)
            b[i].output(s, false);
    }
};

template<class T>
class edabitpack: public pair<FixedVector<T, T::bit_type::default_length>, FixedVector<typename T::bit_type, T::clear::MAX_EDABITS + 5>>
{
    typedef pair<FixedVector<T, T::bit_type::default_length>, FixedVector<typename T::bit_type, T::clear::MAX_EDABITS + 5>> super;
public:

    static const int MAX_SIZE = super::first_type::MAX_SIZE;

    edabitpack()
    {
    }

    edabitpack(const typename super::first_type& a, const typename super::second_type& b) : super(a, b) {

    }

    // edabitpack(const vector<T>& other)
    // {
    //     this->first = other;
    // }

    // edabitpack(const vector<typename T::bit_type>& other)
    // {
    //     this->second = other;
    // }

    void push_a(const T& x)
    {
        this->first.push_back(x);
    }

    void push_b(const typename T::bit_type& x)
    {
        this->second.push_back(x);
    }

    bool empty()
    {
        return this->first.empty();
    }

    bool full()
    {
        return this->first.full();
    }

    size_t size()
    {
        return this->first.size();
    }

    T get_a(int i)
    {
        return this->first[i];
    }

    typename T::bit_type get_b(int i)
    {
        return this->second[i];
    }

    // typename super::first_type::const_iterator a_begin() const {
    //     return this->first.begin();
    // }

    // typename super::first_type::const_iterator a_end() const {
    //     return this->first.end();
    // }

    // void input(int length, ifstream& s)
    // {
    //     char buffer[MAX_SIZE * T::size()];
    //     s.read(buffer, MAX_SIZE * T::size());
    //     for (int i = 0; i < MAX_SIZE; i++)
    //     {
    //         T x;
    //         x.assign(buffer + i * T::size());
    //         a.push_back(x);
    //     }
    //     size_t bsize = T::bit_type::part_type::size();
    //     char bbuffer[length * bsize];
    //     s.read(bbuffer, length * bsize);
    //     for (int i = 0; i < length; i++)
    //     {
    //         typename T::bit_type::part_type x;
    //         x.assign(bbuffer + i * bsize);
    //         b.push_back(x);
    //     }
    // }

    // void output(int length, ofstream& s)
    // {
    //     assert(size() == MAX_SIZE);
    //     for (auto& x : a)
    //         x.output(s, false);
    //     for (int i = 0; i < length; i++)
    //         b[i].output(s, false);
    // }
};


#endif /* PROTOCOLS_EDABIT_H_ */
