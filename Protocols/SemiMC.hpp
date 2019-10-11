/*
 * SemiMC.cpp
 *
 */

#ifndef PROTOCOLS_SEMIMC_HPP_
#define PROTOCOLS_SEMIMC_HPP_

#include "SemiMC.h"

#include "MAC_Check.hpp"

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

template<class T>
void DirectSemiMC<T>::POpen_(vector<typename T::open_type>& values,
        const vector<T>& S, const PlayerBase& P)
{
    values.clear();
    values.insert(values.begin(), S.begin(), S.end());
    Bundle<octetStream> oss(P);
    for (auto& x : values)
        x.pack(oss.mine);
    P.Broadcast_Receive(oss, true);
    direct_add_openings<typename T::open_type, 0>(values, P, oss);
}

template<class T>
void DirectSemiMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    (void) values, (void) S, (void) P;
}

#endif
