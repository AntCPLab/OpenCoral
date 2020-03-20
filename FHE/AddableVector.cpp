/*
 * AddableVector.cpp
 *
 */

#include "AddableVector.h"
#include "Rq_Element.h"
#include "FHE_Keys.h"

template<class T>
AddableVector<T> AddableVector<T>::mul_by_X_i(int j,
        const FHE_PK& pk) const
{
    int phi_m = this->size();
    assert(phi_m == pk.get_params().phi_m());
    AddableVector res(phi_m);
    for (int i = 0; i < phi_m; i++)
    {
        int k = j + i, s = 1;
        while (k >= phi_m)
        {
            k -= phi_m;
            s = -s;
        }
        if (s == 1)
        {
            res[k] = (*this)[i];
        }
        else
        {
            res[k] = -(*this)[i];
        }
    }
    return res;
}

template
AddableVector<fixint<0>> AddableVector<fixint<0>>::mul_by_X_i(int j,
        const FHE_PK& pk) const;
template
AddableVector<fixint<1>> AddableVector<fixint<1>>::mul_by_X_i(int j,
        const FHE_PK& pk) const;
template
AddableVector<fixint<2>> AddableVector<fixint<2>>::mul_by_X_i(int j,
        const FHE_PK& pk) const;
