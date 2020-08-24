/*
 * FakeProtocol.h
 *
 */

#ifndef PROTOCOLS_FAKEPROTOCOL_H_
#define PROTOCOLS_FAKEPROTOCOL_H_

#include "Replicated.h"

template<class T>
class FakeProtocol : public ProtocolBase<T>
{
    PointerVector<T> results;
    SeededPRNG G;

public:
    Player& P;

    FakeProtocol(Player& P) : P(P)
    {
    }

    void init_mul(SubProcessor<T>*)
    {
        results.clear();
    }

    typename T::clear prepare_mul(const T& x, const T& y, int = -1)
    {
        results.push_back(x * y);
        return {};
    }

    void exchange()
    {
    }

    T finalize_mul(int = -1)
    {
        return results.next();
    }

    void randoms(T& res, int n_bits)
    {
        res.randomize_part(G, n_bits);
    }

    int get_n_relevant_players()
    {
        return 1;
    }

    void trunc_pr(const vector<int>& regs, int size, SubProcessor<T>& proc)
    {
        for (size_t i = 0; i < regs.size(); i += 4)
            for (int l = 0; l < size; l++)
                proc.get_S_ref(regs[i] + l) = proc.get_S_ref(regs[i + 1] + l)
                        >> regs[i + 3];
    }
};

#endif /* PROTOCOLS_FAKEPROTOCOL_H_ */
