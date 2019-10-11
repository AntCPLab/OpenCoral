/*
 * MAC_Check_Base.hpp
 *
 */

#ifndef PROTOCOLS_MAC_CHECK_BASE_HPP_
#define PROTOCOLS_MAC_CHECK_BASE_HPP_

#include "MAC_Check_Base.h"

template<class T>
void MAC_Check_Base<T>::POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P)
{
    POpen_Begin(values, S, P);
    POpen_End(values, S, P);
}

template<class T>
typename T::open_type MAC_Check_Base<T>::POpen(const T& secret, const Player& P)
{
    vector<typename T::open_type> opened;
    POpen(opened, {secret}, P);
    return opened[0];
}

template<class T>
void MAC_Check_Base<T>::CheckFor(const typename T::open_type& value,
        const vector<T>& shares, const Player& P)
{
    vector<typename T::open_type> opened;
    POpen(opened, shares, P);
    for (auto& check : opened)
        if (typename T::clear(check) != value)
        {
            cout << check << " != " << value << endl;
            throw Offline_Check_Error("CheckFor");
        }
    Check(P);
}

#endif /* PROTOCOLS_MAC_CHECK_BASE_HPP_ */
