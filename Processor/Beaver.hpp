/*
 * Beaver.cpp
 *
 */

#include "Beaver.h"

#include <array>


template<class T>
void Beaver<T>::init_mul(SubProcessor<T>* proc)
{
    this->proc = proc;
    shares.clear();
    opened.clear();
    triples.clear();
}

template<class T>
typename T::clear Beaver<T>::prepare_mul(const T& x, const T& y)
{
    triples.push_back({{}});
    auto& triple = triples.back();
    proc->DataF.get(DATA_TRIPLE, triple.data());
    shares.push_back(x - triple[0]);
    shares.push_back(y - triple[1]);
    return 0;
}

template<class T>
void Beaver<T>::exchange()
{
    proc->MC.POpen(opened, shares, P);
    it = opened.begin();
    triple = triples.begin();
}

template<class T>
T Beaver<T>::finalize_mul()
{
    typename T::clear masked[2];
    T& tmp = (*triple)[2];
    for (int k = 0; k < 2; k++)
    {
        masked[k] = *it++;
        tmp += (masked[k] * (*triple)[1 - k]);
    }
    tmp.add(tmp, masked[0] * masked[1], P.my_num(), proc->MC.get_alphai());
    triple++;
    return tmp;
}
