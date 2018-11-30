/*
 * Beaver.cpp
 *
 */

#include "Beaver.h"

#include <array>

template<class T>
void Beaver<T>::muls(const vector<int>& reg, SubProcessor<T>& proc, MAC_Check_Base<T>& MC,
        int size)
{
    assert(reg.size() % 3 == 0);
    int n = reg.size() / 3;
    vector<T>& shares = proc.Sh_PO;
    vector<typename T::clear>& opened = proc.PO;
    shares.clear();
    vector<array<T, 3>> triples(n * size);
    auto triple = triples.begin();

    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            proc.DataF.get(DATA_TRIPLE, triple->data());
            for (int k = 0; k < 2; k++)
                shares.push_back(proc.S[reg[i * 3 + k + 1] + j] - (*triple)[k]);
            triple++;
        }

    MC.POpen_Begin(opened, shares, proc.P);
    MC.POpen_End(opened, shares, proc.P);
    auto it = opened.begin();
    triple = triples.begin();

    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            typename T::clear masked[2];
            T& tmp = (*triple)[2];
            for (int k = 0; k < 2; k++)
            {
                masked[k] = *it++;
                tmp += (masked[k] * (*triple)[1 - k]);
            }
            tmp.add(tmp, masked[0] * masked[1], proc.P.my_num(), MC.get_alphai());
            proc.S[reg[i * 3] + j] = tmp;
            triple++;
        }
}
