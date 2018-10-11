/*
 * Multiplication.h
 *
 */

#ifndef PROCESSOR_SPDZ_H_
#define PROCESSOR_SPDZ_H_

#include <vector>
using namespace std;

template<class T> class SubProcessor;
template<class T> class MAC_Check;
template<class T> class Share;

template <class T>
class SPDZ
{
public:
    static void assign(T& share, const T& clear, int my_num)
    {
        if (my_num == 0)
            share = clear;
        else
            share = 0;
    }

    static void muls(const vector<int>& reg, SubProcessor<Share<T> >& proc, MAC_Check<T>& MC,
            int size);
};

#endif /* PROCESSOR_SPDZ_H_ */
