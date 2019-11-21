/*
 * ReplicatedPrep.cpp
 *
 */

#ifndef PROTOCOlS_REPLICATEDPREP_HPP_
#define PROTOCOlS_REPLICATEDPREP_HPP_

#include "ReplicatedPrep.h"
#include "Math/gfp.h"
#include "Processor/OnlineOptions.h"

template<class T>
BufferPrep<T>::BufferPrep(DataPositions& usage) :
        Preprocessing<T>(usage), n_bit_rounds(0),
        buffer_size(OnlineOptions::singleton.batch_size)
{
}

template<class T>
BufferPrep<T>::~BufferPrep()
{
#ifdef VERBOSE
    if (n_bit_rounds > 0)
        cerr << n_bit_rounds << " rounds of random bit generation" << endl;
#endif
}

template<class T>
RingPrep<T>::RingPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage), protocol(0), proc(proc), base_player(0)
{
}

template<class T>
void RingPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    this->protocol = &protocol;
    if (proc)
        base_player = proc->Proc.thread_num;
}

template<class T>
void BufferPrep<T>::clear()
{
    triples.clear();
    inverses.clear();
    bits.clear();
    squares.clear();
    inputs.clear();
}

template<class T>
void ReplicatedRingPrep<T>::buffer_triples()
{
    assert(this->protocol != 0);
    // independent instance to avoid conflicts
    typename T::Protocol protocol(this->protocol->branch());
    generate_triples(this->triples, OnlineOptions::singleton.batch_size,
            &protocol);
}

template<class T, class U>
void generate_triples(vector<array<T, 3>>& triples, int n_triples,
        U* protocol, int n_bits = -1)
{
    triples.resize(n_triples);
    protocol->init_mul();
    for (size_t i = 0; i < triples.size(); i++)
    {
        auto& triple = triples[i];
        triple[0] = protocol->get_random();
        triple[1] = protocol->get_random();
        protocol->prepare_mul(triple[0], triple[1], n_bits);
    }
    protocol->exchange();
    for (size_t i = 0; i < triples.size(); i++)
        triples[i][2] = protocol->finalize_mul(n_bits);
}

template<class T>
void BufferPrep<T>::get_three_no_count(Dtype dtype, T& a, T& b, T& c)
{
    if (dtype != DATA_TRIPLE)
        throw not_implemented();

    if (triples.empty())
    {
        buffer_triples();
        assert(not triples.empty());
    }

    a = triples.back()[0];
    b = triples.back()[1];
    c = triples.back()[2];
    triples.pop_back();
}

template<class T>
void RingPrep<T>::buffer_squares()
{
    auto proc = this->proc;
    auto buffer_size = OnlineOptions::singleton.batch_size;
    assert(proc != 0);
    vector<T> a_plus_b(buffer_size), as(buffer_size), cs(buffer_size);
    T b;
    for (int i = 0; i < buffer_size; i++)
    {
        this->get_three_no_count(DATA_TRIPLE, as[i], b, cs[i]);
        a_plus_b[i] = as[i] + b;
    }
    vector<typename T::open_type> opened(buffer_size);
    proc->MC.POpen(opened, a_plus_b, proc->P);
    for (int i = 0; i < buffer_size; i++)
        this->squares.push_back({{as[i], as[i] * opened[i] - cs[i]}});
}

template<class T>
void ReplicatedRingPrep<T>::buffer_squares()
{
    auto protocol = this->protocol;
    auto proc = this->proc;
    assert(protocol != 0);
    auto& squares = this->squares;
    squares.resize(OnlineOptions::singleton.batch_size);
    protocol->init_mul(proc);
    for (size_t i = 0; i < squares.size(); i++)
    {
        auto& square = squares[i];
        square[0] = protocol->get_random();
        protocol->prepare_mul(square[0], square[0]);
    }
    protocol->exchange();
    for (size_t i = 0; i < squares.size(); i++)
        squares[i][1] = protocol->finalize_mul();
}

template<class T>
void ReplicatedPrep<T>::buffer_inverses()
{
	auto protocol = this->protocol;
    assert(protocol != 0);
    typename T::MAC_Check MC;
    ::buffer_inverses(this->inverses, *this, MC, protocol->P);
}

template<class T>
void buffer_inverses(vector<array<T, 2>>& inverses, Preprocessing<T>& prep,
        MAC_Check_Base<T>& MC, Player& P)
{
    int buffer_size = OnlineOptions::singleton.batch_size;
    vector<array<T, 3>> triples(buffer_size);
    vector<T> c;
    for (int i = 0; i < buffer_size; i++)
    {
        prep.get_three_no_count(DATA_TRIPLE, triples[i][0], triples[i][1],
                triples[i][2]);
        c.push_back(triples[i][2]);
    }
    vector<typename T::open_type> c_open;
    MC.POpen(c_open, c, P);
    for (size_t i = 0; i < c.size(); i++)
        if (c_open[i] != 0)
            inverses.push_back({{triples[i][0], triples[i][1] / c_open[i]}});
    triples.clear();
    if (inverses.empty())
        throw runtime_error("products were all zero");
    MC.Check(P);
}

template<class T>
void BufferPrep<T>::get_two_no_count(Dtype dtype, T& a, T& b)
{
    switch (dtype)
    {
    case DATA_SQUARE:
    {
        if (squares.empty())
            buffer_squares();

        a = squares.back()[0];
        b = squares.back()[1];
        squares.pop_back();
        return;
    }
    case DATA_INVERSE:
    {
        while (inverses.empty())
            buffer_inverses();

        a = inverses.back()[0];
        b = inverses.back()[1];
        inverses.pop_back();
        return;
    }
    default:
        throw not_implemented();
    }
}

template<class T>
void XOR(vector<T>& res, vector<T>& x, vector<T>& y, int buffer_size,
		typename T::Protocol& prot, SubProcessor<T>* proc)
{
    res.resize(buffer_size);

    if (T::clear::field_type() == DATA_GF2N)
    {
        for (int i = 0; i < buffer_size; i++)
            res[i] = x[i] + y[i];
        return;
    }

    prot.init_mul(proc);
    for (int i = 0; i < buffer_size; i++)
        prot.prepare_mul(x[i], y[i]);
    prot.exchange();
    typename T::open_type two = typename T::open_type(1) + typename T::open_type(1);
    for (int i = 0; i < buffer_size; i++)
        res[i] = x[i] + y[i] - prot.finalize_mul() * two;
}

template<template<class U> class T>
void buffer_bits_from_squares(RingPrep<T<gfp>>& prep)
{
    auto proc = prep.get_proc();
    assert(proc != 0);
    auto& bits = prep.get_bits();
    vector<array<T<gfp>, 2>> squares(prep.buffer_size);
    vector<T<gfp>> s;
    for (int i = 0; i < prep.buffer_size; i++)
    {
        prep.get_two(DATA_SQUARE, squares[i][0], squares[i][1]);
        s.push_back(squares[i][1]);
    }
    vector<gfp> open;
    proc->MC.POpen(open, s, proc->P);
    auto one = T<gfp>::constant(1, proc->P.my_num(), proc->MC.get_alphai());
    for (size_t i = 0; i < s.size(); i++)
        if (open[i] != 0)
            bits.push_back((squares[i][0] / open[i].sqrRoot() + one) / 2);
    squares.clear();
    if (bits.empty())
        throw runtime_error("squares were all zero");
}

template<template<class U> class T>
void buffer_bits_spec(ReplicatedPrep<T<gfp>>& prep, vector<T<gfp>>& bits,
    typename T<gfp>::Protocol& prot)
{
    (void) bits, (void) prot;
    if (prot.get_n_relevant_players() > 10)
        buffer_bits_from_squares(prep);
    else
        prep.ReplicatedRingPrep<T<gfp>>::buffer_bits();
}

template<class T>
void RingPrep<T>::buffer_bits_without_check()
{
    assert(protocol != 0);
    auto buffer_size = OnlineOptions::singleton.batch_size;
    auto& bits = this->bits;
    auto& P = protocol->P;
    int n_relevant_players = protocol->get_n_relevant_players();
    vector<vector<T>> player_bits(n_relevant_players, vector<T>(buffer_size));
    typename T::Input input(proc, P);
    input.reset_all(P);
    for (int i = 0; i < n_relevant_players; i++)
    {
        int input_player = (base_player + i) % P.num_players();
        if (input_player == P.my_num())
        {
            SeededPRNG G;
            for (int i = 0; i < buffer_size; i++)
                input.add_mine(G.get_bit());
        }
        else
            for (int i = 0; i < buffer_size; i++)
                input.add_other(input_player);
    }
    input.exchange();
    for (int i = 0; i < n_relevant_players; i++)
        for (auto& x : player_bits[i])
            x = input.finalize((base_player + i) % P.num_players());
    auto& prot = *protocol;
    XOR(bits, player_bits[0], player_bits[1], buffer_size, prot, proc);
    for (int i = 2; i < n_relevant_players; i++)
        XOR(bits, bits, player_bits[i], buffer_size, prot, proc);
    base_player++;
}

template<>
inline
void SemiHonestRingPrep<Rep3Share<gf2n>>::buffer_bits()
{
    assert(protocol != 0);
    for (int i = 0; i < DIV_CEIL(buffer_size, gf2n::degree()); i++)
    {
        Rep3Share<gf2n> share = protocol->get_random();
        for (int j = 0; j < gf2n::degree(); j++)
        {
            bits.push_back(share & 1);
            share >>= 1;
        }
    }
}

template<template<class U> class T>
void buffer_bits_spec(ReplicatedPrep<T<gf2n_short>>& prep, vector<T<gf2n_short>>& bits,
    typename T<gf2n_short>::Protocol& prot)
{
    (void) bits, (void) prot;
    prep.ReplicatedRingPrep<T<gf2n_short>>::buffer_bits();
}

template<template<class U> class T>
void buffer_bits_spec(ReplicatedPrep<T<gf2n_long>>& prep, vector<T<gf2n_long>>& bits,
    typename T<gf2n_long>::Protocol& prot)
{
    (void) bits, (void) prot;
    prep.ReplicatedRingPrep<T<gf2n_long>>::buffer_bits();
}

template<template<class U> class T, int K>
void buffer_bits_spec(ReplicatedPrep<T<Z2<K>>>& prep, vector<T<Z2<K>>>& bits,
    typename T<Z2<K>>::Protocol& prot)
{
    (void) bits, (void) prot;
    prep.ReplicatedRingPrep<T<Z2<K>>>::buffer_bits();
}

template<class T>
void ReplicatedPrep<T>::buffer_bits()
{
    assert(this->protocol != 0);
    buffer_bits_spec(*this, this->bits, *this->protocol);
}

template<class T>
void BufferPrep<T>::get_one_no_count(Dtype dtype, T& a)
{
    if (dtype != DATA_BIT)
        throw not_implemented();

    while (bits.empty())
    {
        buffer_bits();
        n_bit_rounds++;
    }

    a = bits.back();
    bits.pop_back();
}

template<class T>
void BufferPrep<T>::get_input_no_count(T& a, typename T::open_type& x, int i)
{
    (void) a, (void) x, (void) i;
    if (inputs.size() <= (size_t)i or inputs.at(i).empty())
        buffer_inputs(i);
    a = inputs[i].back().share;
    x = inputs[i].back().value;
    inputs[i].pop_back();
}

template<class T>
inline void BufferPrep<T>::buffer_inputs(int player)
{
    (void) player;
    throw not_implemented();
}

template<class T>
void BufferPrep<T>::buffer_inputs_as_usual(int player, SubProcessor<T>* proc)
{
    assert(proc != 0);
    auto& P = proc->P;
    this->inputs.resize(P.num_players());
    typename T::Input input(proc, P);
    input.reset(player);
    auto buffer_size = OnlineOptions::singleton.batch_size;
    if (P.my_num() == player)
    {
        for (int i = 0; i < buffer_size; i++)
        {
            typename T::clear r;
            r.randomize(proc->Proc.secure_prng);
            input.add_mine(r);
            this->inputs[player].push_back({input.finalize_mine(), r});
        }
        input.send_mine();
    }
    else
    {
        octetStream os;
        P.receive_player(player, os, true);
        T share;
        for (int i = 0; i < buffer_size; i++)
        {
            input.finalize_other(player, share, os);
            this->inputs[player].push_back({share, 0});
        }
    }
}

template<class T>
void BufferPrep<T>::get_no_count(vector<T>& S, DataTag tag,
        const vector<int>& regs, int vector_size)
{
    (void) S, (void) tag, (void) regs, (void) vector_size;
    throw not_implemented();
}

#endif
