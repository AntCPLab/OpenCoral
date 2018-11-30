/*
 * Beaver.h
 *
 */

#ifndef PROCESSOR_BEAVER_H_
#define PROCESSOR_BEAVER_H_

#include <vector>
using namespace std;

template<class T> class SubProcessor;
template<class T> class MAC_Check_Base;
class Player;

template<class T>
class Beaver
{
public:
    Player& P;

    static void muls(const vector<int>& reg, SubProcessor<T>& proc,
            MAC_Check_Base<T>& MC, int size);

    Beaver(Player& P) : P(P) {}
};

#endif /* PROCESSOR_BEAVER_H_ */
