/*
 * ReplicatedMC.cpp
 *
 */

#include "ReplicatedMC.h"

template<class T>
void ReplicatedMC<T>::POpen_Begin(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    assert(T::length == 2);
    (void)values;
    o.reset_write_head();
    for (auto& x : S)
        x[0].pack(o);
    P.pass_around(o, -1);
}

template<class T>
void ReplicatedMC<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    (void)P;
    values.resize(S.size());
    for (size_t i = 0; i < S.size(); i++)
    {
        typename T::clear tmp;
        tmp.unpack(o);
        values[i] = S[i].sum() + tmp;
    }
}
