/*
 * ReplicatedPrep.cpp
 *
 */

#include "ReplicatedPrep.h"
#include "Math/gfp.h"
#include "Math/MaliciousRep3Share.h"
#include "Auth/ReplicatedMC.h"
#include "Auth/ShamirMC.h"

template<class T>
ReplicatedRingPrep<T>::ReplicatedRingPrep(SubProcessor<T>* proc) :
        protocol(0), proc(proc)
{
}

template<class T>
void ReplicatedRingPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    this->protocol = &protocol;
    if (proc)
        base_player = proc->Proc.thread_num;
    else
        base_player = 0;
}

template<class T>
void ReplicatedRingPrep<T>::buffer_triples()
{
    assert(protocol != 0);
    auto& triples = this->triples;
    triples.resize(this->buffer_size);
    protocol->init_mul(proc);
    for (size_t i = 0; i < triples.size(); i++)
    {
        auto& triple = triples[i];
        triple[0] = protocol->get_random();
        triple[1] = protocol->get_random();
        protocol->prepare_mul(triple[0], triple[1]);
    }
    protocol->exchange();
    for (size_t i = 0; i < triples.size(); i++)
        triples[i][2] = protocol->finalize_mul();
}

template<class T>
void BufferPrep<T>::get_three(Dtype dtype, T& a, T& b, T& c)
{
    if (dtype != DATA_TRIPLE)
        throw not_implemented();

    if (triples.empty())
        buffer_triples();

    a = triples.back()[0];
    b = triples.back()[1];
    c = triples.back()[2];
    triples.pop_back();
}

template<class T>
void ReplicatedRingPrep<T>::buffer_squares()
{
    assert(protocol != 0);
    auto& squares = this->squares;
    squares.resize(this->buffer_size);
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
    BufferPrep<T>::buffer_inverses(MC, protocol->P);
}

template<class T>
void BufferPrep<T>::buffer_inverses(MAC_Check_Base<T>& MC, Player& P)
{
    vector<array<T, 3>> triples(buffer_size);
    vector<T> c;
    for (int i = 0; i < buffer_size; i++)
    {
        get_three(DATA_TRIPLE, triples[i][0], triples[i][1], triples[i][2]);
        c.push_back(triples[i][2]);
    }
    vector<typename T::clear> c_open;
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
void BufferPrep<T>::get_two(Dtype dtype, T& a, T& b)
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
    prot.init_mul(proc);
    for (int i = 0; i < buffer_size; i++)
        prot.prepare_mul(x[i], y[i]);
    prot.exchange();
    res.resize(buffer_size);
    typename T::clear two = typename T::clear(1) + typename T::clear(1);
    for (int i = 0; i < buffer_size; i++)
        res[i] = x[i] + y[i] - prot.finalize_mul() * two;
}

template<template<class U> class T>
void buffer_bits_spec(ReplicatedPrep<T<gfp>>& prep, vector<T<gfp>>& bits,
    typename T<gfp>::Protocol& prot)
{
    (void) bits, (void) prot;
    if (prot.get_n_relevant_players() > 10)
    {
        vector<array<T<gfp>, 2>> squares(prep.buffer_size);
        vector<T<gfp>> s;
        for (int i = 0; i < prep.buffer_size; i++)
        {
            prep.get_two(DATA_SQUARE, squares[i][0], squares[i][1]);
            s.push_back(squares[i][1]);
        }
        vector<gfp> open;
        typename T<gfp>::MAC_Check().POpen(open, s, prot.P);
        auto one = T<gfp>(1, prot.P.my_num());
        for (size_t i = 0; i < s.size(); i++)
            if (open[i] != 0)
                bits.push_back((squares[i][0] / open[i].sqrRoot() + one) / 2);
        squares.clear();
        if (bits.empty())
            throw runtime_error("squares were all zero");
    }
    else
        prep.ReplicatedRingPrep<T<gfp>>::buffer_bits();
}

template<class T>
void ReplicatedRingPrep<T>::buffer_bits()
{
    assert(protocol != 0);
    auto buffer_size = this->buffer_size;
    auto& bits = this->bits;
    auto& P = protocol->P;
    int n_relevant_players = protocol->get_n_relevant_players();
    vector<vector<T>> player_bits(n_relevant_players, vector<T>(buffer_size));
    typename T::Input input(proc, P);
    for (int i = 0; i < P.num_players(); i++)
        input.reset(i);
    if (positive_modulo(P.my_num() - base_player, P.num_players()) < n_relevant_players)
    {
        SeededPRNG G;
        for (int i = 0; i < buffer_size; i++)
            input.add_mine(G.get_bit());
        input.send_mine();
    }
    for (int i = 0; i < n_relevant_players; i++)
    {
        int input_player = (base_player + i) % P.num_players();
        if (input_player == P.my_num())
            for (auto& x : player_bits[i])
                x = input.finalize_mine();
        else
        {
            octetStream os;
            P.receive_player(input_player, os, true);
            for (auto& x : player_bits[i])
                input.finalize_other(input_player, x, os);
        }
    }
    auto& prot = *protocol;
    XOR(bits, player_bits[0], player_bits[1], buffer_size, prot, proc);
    for (int i = 2; i < n_relevant_players; i++)
        XOR(bits, bits, player_bits[i], buffer_size, prot, proc);
    base_player++;
}

template<>
inline
void ReplicatedRingPrep<Rep3Share<gf2n>>::buffer_bits()
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
void buffer_bits_spec(ReplicatedPrep<T<gf2n>>& prep, vector<T<gf2n>>& bits,
    typename T<gf2n>::Protocol& prot)
{
    (void) bits, (void) prot;
    prep.ReplicatedRingPrep<T<gf2n>>::buffer_bits();
}

template<class T>
void ReplicatedPrep<T>::buffer_bits()
{
    assert(this->protocol != 0);
    buffer_bits_spec(*this, this->bits, *this->protocol);
}

template<class T>
void BufferPrep<T>::get_one(Dtype dtype, T& a)
{
    if (dtype != DATA_BIT)
        throw not_implemented();

    while (bits.empty())
        buffer_bits();

    a = bits.back();
    bits.pop_back();
}

template<class T>
void BufferPrep<T>::get_input(T& a, typename T::clear& x, int i)
{
    (void) a, (void) x, (void) i;
    throw not_implemented();
}

template<class T>
void BufferPrep<T>::get(vector<T>& S, DataTag tag,
        const vector<int>& regs, int vector_size)
{
    (void) S, (void) tag, (void) regs, (void) vector_size;
    throw not_implemented();
}
