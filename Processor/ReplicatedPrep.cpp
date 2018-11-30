/*
 * ReplicatedPrep.cpp
 *
 */

#include "ReplicatedPrep.h"
#include "Math/gfp.h"
#include "Math/MaliciousRep3Share.h"
#include "Auth/ReplicatedMC.h"

template<class T>
ReplicatedPrep<T>::ReplicatedPrep() : protocol(0)
{
}

template<class T>
void ReplicatedPrep<T>::buffer_triples()
{
    assert(protocol != 0);
    auto& triples = this->triples;
    triples.resize(this->buffer_size);
    protocol->init_mul();
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
void ReplicatedPrep<T>::buffer_squares()
{
    assert(protocol != 0);
    auto& squares = this->squares;
    squares.resize(this->buffer_size);
    protocol->init_mul();
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
    assert(protocol != 0);
    ReplicatedMC<T> MC;
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
            inverses.push_back({triples[i][0], triples[i][1] / c_open[i]});
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

template<>
void ReplicatedPrep<Rep3Share<gfp>>::buffer_bits()
{
    assert(protocol != 0);
#ifdef BIT_BY_SQUARE
    vector<array<Rep3Share<gfp>, 2>> squares(buffer_size);
    vector<Rep3Share<gfp>> s;
    for (int i = 0; i < buffer_size; i++)
    {
        get_two(DATA_SQUARE, squares[i][0], squares[i][1]);
        s.push_back(squares[i][1]);
    }
    vector<gfp> open;
    ReplicatedMC<Rep3Share<gfp>>().POpen(open, s, protocol->P);
    auto one = Rep3Share<gfp>(1, protocol->P.my_num());
    for (size_t i = 0; i < s.size(); i++)
        if (open[i] != 0)
            bits.push_back((squares[i][0] / open[i].sqrRoot() + one) / 2);
    squares.clear();
    if (bits.empty())
        throw runtime_error("squares were all zero");
#else
    vector<vector<Rep3Share<gfp>>> player_bits(3, vector<Rep3Share<gfp>>(buffer_size));
    vector<octetStream> os(2);
    SeededPRNG G;
    for (auto& share : player_bits[protocol->P.my_num()])
    {
        share.randomize_to_sum(G.get_bit(), G);
        for (int i = 0; i < 2; i++)
            share[i].pack(os[i]);
    }
    auto& prot = *protocol;
    prot.P.send_relative(os);
    prot.P.receive_relative(os);
    for (int i = 0; i < 2; i++)
        for (auto& share : player_bits[prot.P.get_player(i + 1)])
            share[i].unpack(os[i]);
    prot.init_mul();
    for (int i = 0; i < buffer_size; i++)
        prot.prepare_mul(player_bits[0][i], player_bits[1][i]);
    prot.exchange();
    vector<Rep3Share<gfp>> first_xor(buffer_size);
    gfp two(2);
    for (int i = 0; i < buffer_size; i++)
        first_xor[i] = player_bits[0][i] + player_bits[1][i] - prot.finalize_mul() * two;
    prot.init_mul();
    for (int i = 0; i < buffer_size; i++)
        prot.prepare_mul(player_bits[2][i], first_xor[i]);
    prot.exchange();
    bits.resize(buffer_size);
    for (int i = 0; i < buffer_size; i++)
        bits[i] = player_bits[2][i] + first_xor[i] - prot.finalize_mul() * two;
#endif
}

template<>
void ReplicatedPrep<Rep3Share<gf2n>>::buffer_bits()
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

template class BufferPrep<Rep3Share<gfp>>;
template class BufferPrep<Rep3Share<gf2n>>;
template class BufferPrep<MaliciousRep3Share<gfp>>;
template class BufferPrep<MaliciousRep3Share<gf2n>>;
template class ReplicatedPrep<Rep3Share<gfp>>;
template class ReplicatedPrep<Rep3Share<gf2n>>;
