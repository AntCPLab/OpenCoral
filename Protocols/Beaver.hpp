/*
 * Beaver.cpp
 *
 */

#include "Beaver.h"

#include "Replicated.hpp"

#include <array>


template<class T>
void Beaver<T>::init_mul(SubProcessor<T>* proc)
{
    assert(proc != 0);
    init_mul(proc->DataF, proc->MC);
}

template<class T>
void Beaver<T>::init_mul(Preprocessing<T>& prep, typename T::MAC_Check& MC)
{
    this->prep = &prep;
    this->MC = &MC;
    shares.clear();
    opened.clear();
    triples.clear();
}

template<class T>
typename T::clear Beaver<T>::prepare_mul(const T& x, const T& y, int n)
{
    (void) n;
    triples.push_back({{}});
    auto& triple = triples.back();
    prep->get(DATA_TRIPLE, triple.data());
    shares.push_back(x - triple[0]);
    shares.push_back(y - triple[1]);
    return 0;
}

template<class T>
void Beaver<T>::exchange()
{
    MC->POpen(opened, shares, P);
    it = opened.begin();
    triple = triples.begin();
}

template<class T>
T Beaver<T>::finalize_mul(int n)
{
    (void) n;
    typename T::clear masked[2];
    T& tmp = (*triple)[2];
    for (int k = 0; k < 2; k++)
    {
        masked[k] = *it++;
        tmp += (masked[k] * (*triple)[1 - k]);
    }
    tmp += T::constant(masked[0] * masked[1], P.my_num(), MC->get_alphai());
    triple++;
    return tmp;
}
