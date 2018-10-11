/*
 * Replicated.h
 *
 */

#ifndef PROCESSOR_REPLICATED_H_
#define PROCESSOR_REPLICATED_H_

#include <assert.h>
#include <vector>
using namespace std;

#include "Tools/octetStream.h"

template<class T> class SubProcessor;
template<class T> class ReplicatedMC;
template<class T> class ReplicatedInput;
template<class T> class ReplicatedPrivateOutput;
template<class T> class Share;

template <class T>
class Replicated
{
    vector<octetStream> os;
    vector<T> results;
    int counter;

public:
    typedef ReplicatedMC<T> MAC_Check;
    typedef ReplicatedInput<T> Input;
    typedef ReplicatedPrivateOutput<T> PrivateOutput;

    Replicated();
    ~Replicated();

    static void assign(T& share, const typename T::value_type& value, int my_num)
    {
        assert(T::length == 2);
        share.assign_zero();
        if (my_num < 2)
            share[my_num] = value;
    }

    void muls(const vector<int>& reg, SubProcessor<Share<T> >& proc, ReplicatedMC<T>& MC,
            int size);
};

#endif /* PROCESSOR_REPLICATED_H_ */
