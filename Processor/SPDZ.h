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
class Player;
template<class sint> class Processor;

template <class T>
class SPDZ
{
public:
    SPDZ(Player& P)
    {
        (void) P;
    }

    static void assign(T& share, const T& clear, int my_num)
    {
        if (my_num == 0)
            share = clear;
        else
            share = 0;
    }

    static void muls(const vector<int>& reg, SubProcessor<Share<T> >& proc, MAC_Check<T>& MC,
            int size);

    static void reqbl(int n);

    static void input(SubProcessor<Share<T>>& Proc, int n, int* r);
};

#endif /* PROCESSOR_SPDZ_H_ */
