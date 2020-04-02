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
        it += T::n;
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
    static const int n = 4;

    int from;
    int& n_bits;
    int& n_shift;
    int params[2];
    int dest;

    InputArgs(vector<int>::const_iterator it) : n_bits(params[0]), n_shift(params[1])
    {
        from = *it++;
        n_bits = *it++;
        n_shift = *it++;
        dest = *it++;
    }
};

class InputArgList : public ArgList<InputArgs>
{
public:
    InputArgList(const vector<int>& args) :
            ArgList<InputArgs>(args)
    {
    }

    int n_inputs_from(int from)
    {
        int res = 0;
        for (auto x : *this)
            res += x.from == from;
        return res;
    }

    int n_interactive_inputs_from_me(int my_num);
};

#endif /* GC_ARGTUPLES_H_ */
