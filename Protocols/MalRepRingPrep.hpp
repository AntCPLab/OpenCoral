/*
 * MalRepRingPrep.cpp
 *
 */

#include "MalRepRingPrep.h"
#include "MaliciousRepPrep.h"
#include "MalRepRingOptions.h"
#include "ShuffleSacrifice.h"
#include "Processor/OnlineOptions.h"

template<class T>
MalRepRingPrep<T>::MalRepRingPrep(SubProcessor<T>* proc,
        DataPositions& usage) : MaliciousRingPrep<T>(proc, usage)
{
}

template<class T>
MalRepRingPrepWithBits<T>::MalRepRingPrepWithBits(SubProcessor<T>* proc,
        DataPositions& usage) : MalRepRingPrep<T>(proc, usage)
{
}

template<class T>
void MalRepRingPrep<T>::buffer_triples()
{
    if (MalRepRingOptions::singleton.shuffle)
        shuffle_buffer_triples();
    else
        simple_buffer_triples();
}

template<class T>
void MalRepRingPrep<T>::buffer_squares()
{
    typedef typename T::prep_type prep_type;
    DataPositions _;
    MaliciousRepPrep<prep_type> prep(_);
    assert(this->proc != 0);
    prep.init_honest(this->proc->P);
    prep.buffer_squares();
    for (auto& x : prep.squares)
        this->squares.push_back({{x[0], x[1]}});
}

template<class T>
void MalRepRingPrep<T>::simple_buffer_triples()
{
    typedef typename T::prep_type prep_type;
    DataPositions _;
    MaliciousRepPrep<prep_type> prep(_);
    assert(this->proc != 0);
    prep.init_honest(this->proc->P);
    prep.buffer_triples();
    for (auto& x : prep.triples)
        this->triples.push_back({{x[0], x[1], x[2]}});
}

template<class T>
void MalRepRingPrep<T>::shuffle_buffer_triples()
{
    assert(T::SECURITY <= 40);
    assert(this->proc != 0);
    typename T::MAC_Check MC;
    shuffle_triple_generation(this->triples, this->proc->P, MC);
}

template<class T>
void shuffle_triple_generation(vector<array<T, 3>>& triples, Player& P,
        typename T::MAC_Check& MC, int n_bits = -1)
{
    ShuffleSacrifice<T> sacrifice;
    vector<array<T, 3>> check_triples;
    int buffer_size = sacrifice.minimum_n_inputs(OnlineOptions::singleton.batch_size);

    // optimistic triple generation
    Replicated<T> protocol(P);
    generate_triples(check_triples, buffer_size, &protocol, n_bits);

    sacrifice.triple_sacrifice(triples, check_triples, P, MC);
}

template<class T>
void ShuffleSacrifice<T>::triple_sacrifice(vector<array<T, 3>>& triples,
        vector<array<T, 3>>& check_triples, Player& P,
        typename T::MAC_Check& MC)
{
    int buffer_size = check_triples.size();
    assert(buffer_size >= minimum_n_inputs());
    int N = (buffer_size - C) / B;

    // shuffle
    GlobalPRNG G(P);
    for (int i = 0; i < buffer_size; i++)
    {
        int remaining = buffer_size - i;
        int pos = G.get_uint(remaining);
        swap(check_triples[i], check_triples[i + pos]);
    }

    // opening C triples
    vector<T> shares;
    for (int i = 0; i < C; i++)
    {
        for (int j = 0; j < 3; j++)
            shares.push_back(check_triples.back()[j]);
        check_triples.pop_back();
    }
    vector<typename T::open_type> opened;
    MC.POpen(opened, shares, P);
    for (int i = 0; i < C; i++)
        if (typename T::clear(opened[3 * i] * opened[3 * i + 1]) != opened[3 * i + 2])
            throw Offline_Check_Error("shuffle opening");

    // sacrifice buckets
    vector<T> masked;
    masked.reserve(2 * N);
    for (int i = 0; i < N; i++)
    {
        T& a = check_triples[i][0];
        T& b = check_triples[i][1];
        for (int j = 1; j < C; j++)
        {
            T& f = check_triples[i + N * j][0];
            T& g = check_triples[i + N * j][1];
            masked.push_back(a - f);
            masked.push_back(b - g);
        }
    }
    MC.POpen(opened, masked, P);
    auto it = opened.begin();
    vector<T> checks;
    checks.reserve(2 * N);
    for (int i = 0; i < N; i++)
    {
        T& b = check_triples[i][1];
        T& c = check_triples[i][2];
        for (int j = 1; j < C; j++)
        {
            T& f = check_triples[i + N * j][0];
            T& h = check_triples[i + N * j][2];
            typename T::open_type& rho = *(it++);
            typename T::open_type& sigma = *(it++);
            checks.push_back(c - h - b * rho - f * sigma);
        }
    }
    MC.CheckFor(0, checks, P);
    check_triples.resize(N);
    triples = check_triples;
}

template<class T>
void MalRepRingPrepWithBits<T>::buffer_bits()
{
    auto proc = this->proc;
    assert(proc != 0);
    typedef MalRepRingShare<T::BIT_LENGTH + 2, T::SECURITY> BitShare;
    typename BitShare::MAC_Check MC;
    DataPositions usage;
    MalRepRingPrep<BitShare> prep(0, usage);
    SubProcessor<BitShare> bit_proc(proc->Proc, MC, prep, proc->P);
    prep.set_proc(&bit_proc);
    bits_from_square_in_ring(this->bits, OnlineOptions::singleton.batch_size, &prep);
}

template<class T>
void MalRepRingPrep<T>::buffer_inputs(int player)
{
    this->buffer_inputs_as_usual(player, this->proc);
}
