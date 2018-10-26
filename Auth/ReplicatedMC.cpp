/*
 * ReplicatedMC.cpp
 *
 */

#include "ReplicatedMC.h"
#include "GC/ReplicatedSecret.h"
#include "Math/Rep3Share.h"

template<class T>
void ReplicatedMC<T>::POpen_Begin(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    assert(T::length == 2);
    (void)values;
    octetStream o;
    for (auto& x : S)
        x[0].pack(o);
    P.send_relative(-1, o);
}

template<class T>
void ReplicatedMC<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    octetStream o;
    P.receive_relative(1, o);
    values.resize(S.size());
    for (size_t i = 0; i < S.size(); i++)
    {
        typename T::clear tmp;
        tmp.unpack(o);
        values[i] = S[i].sum() + tmp;
    }
}

template class ReplicatedMC<Rep3Share>;
template class ReplicatedMC<GC::ReplicatedSecret>;
