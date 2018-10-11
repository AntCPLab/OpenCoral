/*
 * Multiplication.cpp
 *
 */

#include "SPDZ.h"
#include "Processor.h"
#include "Math/Share.h"

template<class T>
void SPDZ<T>::muls(const vector<int>& reg, SubProcessor<Share<T> >& proc, MAC_Check<T>& MC,
        int size)
{
    assert(reg.size() % 3 == 0);
    int n = reg.size() / 3;
    vector<Share<T> >& shares = proc.Sh_PO;
    vector<T>& opened = proc.PO;
    shares.clear();
    Share<T> triples[n][size][3];

    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            proc.Proc.DataF.get(DATA_TRIPLE, triples[i][j]);
            for (int k = 0; k < 2; k++)
                shares.push_back(proc.S[reg[i * 3 + k + 1] + j] - triples[i][j][k]);
        }

    MC.POpen_Begin(opened, shares, proc.P);
    MC.POpen_End(opened, shares, proc.P);
    auto it = opened.begin();

    for (int i = 0; i < n; i++)
        for (int j = 0; j < size; j++)
        {
            T masked[2];
            Share<T>& tmp = triples[i][j][2];
            for (int k = 0; k < 2; k++)
            {
                masked[k] = *it++;
                tmp.add(masked[k] * triples[i][j][1 - k]);

            }
            tmp.add(tmp, masked[0] * masked[1], proc.P.my_num(), MC.get_alphai());
            proc.S[reg[i * 3] + j] = tmp;
        }
}

#ifndef REPLICATED
template class SPDZ<gfp>;
#endif

template class SPDZ<gf2n>;
