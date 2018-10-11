/*
 * ReplicatedMC.cpp
 *
 */

#include "ReplicatedMC.h"
#include "GC/ReplicatedSecret.h"

template<class T>
void ReplicatedMC<T>::POpen_Begin(vector<typename T::value_type>& values,
        const vector<Share<T> >& S, const Player& P)
{
    assert(T::length == 2);
    (void)values;
    octetStream o;
    for (auto& x : S)
        x.get_share()[0].pack(o);
    P.send_relative(-1, o);
}

template<class T>
void ReplicatedMC<T>::POpen_End(vector<typename T::value_type>& values,
        const vector<Share<T> >& S, const Player& P)
{
    octetStream o;
    P.receive_relative(1, o);
    values.resize(S.size());
    for (size_t i = 0; i < S.size(); i++)
    {
        typename T::value_type tmp;
        tmp.unpack(o);
        values[i] = S[i].get_share().sum() + tmp;
    }
}

template class ReplicatedMC<FixedVec<Integer, 2> >;
template class ReplicatedMC<GC::ReplicatedSecret>;
