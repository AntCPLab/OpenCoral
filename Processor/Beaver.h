/*
 * Beaver.h
 *
 */

#ifndef PROCESSOR_BEAVER_H_
#define PROCESSOR_BEAVER_H_

#include <vector>
#include <array>
using namespace std;

#include "Replicated.h"

template<class T> class SubProcessor;
template<class T> class MAC_Check_Base;
class Player;

template<class T>
class Beaver : public ProtocolBase<T>
{
    vector<T> shares;
    vector<typename T::clear> opened;
    vector<array<T, 3>> triples;
    typename vector<typename T::clear>::iterator it;
    typename vector<array<T, 3>>::iterator triple;
    SubProcessor<T>* proc;

public:
    Player& P;

    Beaver(Player& P) : proc(0), P(P) {}

    void init_mul(SubProcessor<T>* proc);
    typename T::clear prepare_mul(const T& x, const T& y);
    void exchange();
    T finalize_mul();
};

#endif /* PROCESSOR_BEAVER_H_ */
