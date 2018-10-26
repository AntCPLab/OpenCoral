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
            proc.DataF.get(DATA_TRIPLE, triples[i][j]);
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

template<>
void SPDZ<gfp>::reqbl(int n)
{
    if ((int)n > 0 && gfp::pr() < bigint(1) << (n-1))
    {
        cout << "Tape requires prime of bit length " << n << endl;
        throw invalid_params();
    }
    else if ((int)n < 0)
    {
        throw Processor_Error("Program compiled for rings not fields");
    }
}

template<class T>
inline void SPDZ<T>::input(SubProcessor<Share<T>>& Proc, int n, int* r)
{
    T rr, t, tmp;
    Proc.DataF.get_input(Proc.get_S_ref(r[0]),rr,n);
    octetStream o;
    if (n==Proc.P.my_num())
    {
        T xi;
#ifdef DEBUG
        printf("Enter your input : \n");
#endif
        long x;
        cin >> x;
        t.assign(x);
        t.sub(t,rr);
        t.pack(o);
        Proc.P.send_all(o);
        xi.add(t,Proc.get_S_ref(r[0]).get_share());
        Proc.get_S_ref(r[0]).set_share(xi);
    }
    else
    {
        Proc.P.receive_player(n,o);
        t.unpack(o);
    }
    tmp.mul(t, Proc.MC.get_alphai());
    tmp.add(Proc.get_S_ref(r[0]).get_mac(),tmp);
    Proc.get_S_ref(r[0]).set_mac(tmp);
}

template class SPDZ<gfp>;
template class SPDZ<gf2n>;
