/*
 * ArgTuples.h
 *
 */

#ifndef GC_ARGTUPLES_H_
#define GC_ARGTUPLES_H_

#include <vector>
using namespace std;

template <class T>
class ArgIter
{
    vector<int>::const_iterator it;

public:
    ArgIter(const vector<int>::const_iterator it) : it(it)
    {
    }

    T operator*()
    {
        return it;
    }

    ArgIter<T> operator++()
    {
        auto res = it;
        it += 3;
        return res;
    }

    bool operator!=(const ArgIter<T>& other)
    {
        return it != other.it;
    }
};

template <class T>
class ArgList
{
    const vector<int>& args;

public:
    ArgList(const vector<int>& args) :
            args(args)
    {
        if (args.size() % T::n != 0)
            throw runtime_error("wrong number of args");
    }

    ArgIter<T> begin()
    {
        return args.begin();
    }

    ArgIter<T> end()
    {
        return args.end();
    }
};

class InputArgs
{
public:
    static const int n = 3;

    int from;
    int n_bits;
    int dest;

    InputArgs(vector<int>::const_iterator it)
    {
        from = *it++;
        n_bits = *it++;
        dest = *it++;
    }
};



#endif /* GC_ARGTUPLES_H_ */
