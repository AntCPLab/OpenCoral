/*
 * ReplicatedMC.cpp
 *
 */

#ifndef PROTOCOLS_REPLICATEDMC_HPP_
#define PROTOCOLS_REPLICATEDMC_HPP_

#include "ReplicatedMC.h"

template<class T>
void ReplicatedMC<T>::POpen_Begin(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    assert(T::length == 2);
    (void)values;
    o.reset_write_head();
    to_send.reset_write_head();
    for (auto& x : S)
        x[0].pack(to_send);
    P.pass_around(to_send, o, -1);
}

template<class T>
void ReplicatedMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    (void)P;
    values.resize(S.size());
    for (size_t i = 0; i < S.size(); i++)
    {
        typename T::open_type tmp;
        tmp.unpack(o);
        values[i] = S[i].sum() + tmp;
    }
}

#endif
