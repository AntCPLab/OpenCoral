/*
 * FixedVector.h
 *
 */

#ifndef TOOLS_FIXEDVECTOR_H_
#define TOOLS_FIXEDVECTOR_H_

#include <array>
#include <vector>
#include <assert.h>
using namespace std;

template<class T, size_t L>
class FixedVector : array<T, L>
{
    size_t used_size;

public:
    FixedVector()
    {
        used_size = 0;
    }

    FixedVector(const vector<T>& other)
    {
        reserve(other.size());
        std::copy(other.begin(), other.end(), this->begin());
        used_size = other.size();
    }

    void reserve(size_t size)
    {
        if (size > L)
            throw runtime_error("too large: " + to_string(size));
    }

    void push_back(const T& x)
    {
        assert(used_size < L);
        (*this)[used_size] = x;
        used_size++;
    }

    T pop()
    {
        assert(used_size > 0);
        return (*this)[--used_size];
    }

    bool empty()
    {
        return used_size == 0;
    }

    size_t size()
    {
        return used_size;
    }

    void resize(size_t size)
    {
        used_size = size;
    }

    typename array<T, L>::iterator begin()
    {
        return array<T, L>::begin();
    }

    typename array<T, L>::iterator end()
    {
        return begin() + used_size;
    }

    typename array<T, L>::const_iterator begin() const
    {
        return array<T, L>::begin();
    }

    typename array<T, L>::const_iterator end() const
    {
        return begin() + used_size;
    }

    T& operator[](size_t i)
    {
        return array<T, L>::operator[](i);
    }
};

#endif /* TOOLS_FIXEDVECTOR_H_ */
