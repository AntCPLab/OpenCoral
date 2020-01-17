/*
 * ShuffleSacrifice.hpp
 *
 */

#ifndef PROTOCOLS_SHUFFLESACRIFICE_HPP_
#define PROTOCOLS_SHUFFLESACRIFICE_HPP_

#include "ShuffleSacrifice.h"

#include "MalRepRingPrep.hpp"

template<class T>
inline void ShuffleSacrifice<T>::triple_combine(vector<array<T, 3> >& triples,
        vector<array<T, 3> >& to_combine, Player& P,
        typename T::MAC_Check& MC)
{
    int buffer_size = to_combine.size();
    int N = buffer_size / B;
    assert(minimum_n_outputs() <= N);

    shuffle(to_combine, P);

    vector<typename T::open_type> opened;
    vector<T> masked;
    masked.reserve(N);
    for (int i = 0; i < N; i++)
    {
        T& b = to_combine[i][1];
        for (int j = 1; j < B; j++)
        {
            T& g = to_combine[i + N * j][1];
            masked.push_back(b - g);
        }
    }
    MC.POpen(opened, masked, P);
    auto it = opened.begin();
    for (int i = 0; i < N; i++)
    {
        T& a = to_combine[i][0];
        T& c = to_combine[i][2];
        for (int j = 1; j < B; j++)
        {
            T& f = to_combine[i + N * j][0];
            T& h = to_combine[i + N * j][2];
            auto& rho = *(it++);
            a += f;
            c += h + f * rho;
        }
    }
    to_combine.resize(N);
    triples = to_combine;
}

template<class T>
void ShuffleSacrifice<T>::dabit_sacrifice(vector<dabit<T> >& output,
        vector<dabit<T> >& to_check, SubProcessor<T>& proc)
{
    auto& P = proc.P;
    auto& MC = proc.MC;

    int buffer_size = to_check.size();
    int N = (buffer_size - C) / B;

    shuffle(to_check, P);

    // opening C
    vector<T> shares;
    vector<typename T::bit_type::part_type> bit_shares;
    for (int i = 0; i < C; i++)
    {
        shares.push_back(to_check.back().first);
        bit_shares.push_back(to_check.back().second);
        to_check.pop_back();
    }
    vector<typename T::open_type> opened;
    MC.POpen(opened, shares, P);
    vector<typename T::bit_type::part_type::open_type> bits;
    auto& MCB = *T::bit_type::part_type::new_mc(
            GC::ShareThread<typename T::bit_type>::s().MC->get_alphai());
    MCB.POpen(bits, bit_shares, P);
    for (int i = 0; i < C; i++)
        if (opened[i] != bits[i].get())
            throw Offline_Check_Error("dabit shuffle opening");

    // sacrifice buckets
    typename T::Protocol protocol(P);
    protocol.init_mul(&proc);
    for (int i = 0; i < N; i++)
    {
        auto& a = to_check[i].first;
        for (int j = 1; j < B; j++)
        {
            auto& f = to_check[i + N * j].first;
            protocol.prepare_mul(a, f);
        }
    }
    protocol.exchange();
    shares.clear();
    bit_shares.clear();
    shares.reserve((B - 1) * N);
    bit_shares.reserve((B - 1) * N);
    for (int i = 0; i < N; i++)
    {
        auto& a = to_check[i].first;
        auto& b = to_check[i].second;
        for (int j = 1; j < B; j++)
        {
            auto& f = to_check[i + N * j].first;
            auto& g = to_check[i + N * j].second;
            shares.push_back(a + f - protocol.finalize_mul() * 2);
            bit_shares.push_back(b + g);
        }
    }
    MC.POpen(opened, shares, P);
    MCB.POpen(bits, bit_shares, P);
    for (int i = 0; i < (B - 1) * N; i++)
        if (opened[i] != bits[i].get())
            throw Offline_Check_Error("dabit shuffle opening");

    to_check.resize(N);
    output = to_check;
    MCB.Check(P);
    delete &MCB;
}

#endif /* PROTOCOLS_SHUFFLESACRIFICE_HPP_ */
