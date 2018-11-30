/*
 * Multiplication.h
 *
 */

#ifndef PROCESSOR_SPDZ_H_
#define PROCESSOR_SPDZ_H_

#include "Beaver.h"

#include <vector>
using namespace std;

template<class T> class SubProcessor;
template<class T> class MAC_Check;
template<class T> class Share;
class Player;

template <class T>
class SPDZ : public Beaver<Share<T>>
{
public:
    SPDZ(Player& P) : Beaver<Share<T>>(P)
    {
    }

    static void assign(T& share, const T& clear, int my_num)
    {
        if (my_num == 0)
            share = clear;
        else
            share = 0;
    }
};

#endif /* PROCESSOR_SPDZ_H_ */
