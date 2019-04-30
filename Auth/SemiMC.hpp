/*
 * SemiMC.cpp
 *
 */

#include "SemiMC.h"

template<class T>
void SemiMC<T>::POpen_Begin(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    values.clear();
    for (auto& x : S)
        values.push_back(x);
    this->start(values, P);
}

template<class T>
void SemiMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    (void) S;
    this->finish(values, P);
}
