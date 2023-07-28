/*
 * RmfeSharePrep.cpp
 *
 */

#ifndef GC_RMFESHARE_PREP_HPP_
#define GC_RMFESHARE_PREP_HPP_

#include "RmfeSharePrep.h"

#include "PersonalPrep.hpp"
#include "Tools/debug.h"

namespace GC
{

template<class T>
RmfeSharePrep<T>::RmfeSharePrep(DataPositions& usage, int input_player) :
        PersonalPrep<T>(usage, input_player),
        triple_generator(0)
        // real_triple_generator(0)
{
    prng.SetSeed((const unsigned char*) "insecure");
}

template<class T>
RmfeSharePrep<T>::RmfeSharePrep(SubProcessor<T>*, DataPositions& usage) :
        RmfeSharePrep(usage)
{
}

template<class T>
RmfeSharePrep<T>::~RmfeSharePrep()
{
    if (triple_generator)
        delete triple_generator;
    // if (real_triple_generator)
    //     delete real_triple_generator;
}

template<class T>
void RmfeSharePrep<T>::set_protocol(typename T::Protocol& protocol)
{
    if (triple_generator)
    {
        assert(&triple_generator->get_player() == &protocol.P);
        return;
    }

    params.generateMACs = true;
    params.amplify = false;
    params.check = false;
    auto& thread = ShareThread<typename T::whole_type>::s();
    triple_generator = new typename T::TripleGenerator(
            BaseMachine::fresh_ot_setup(protocol.P), protocol.P.N, -1,
            OnlineOptions::singleton.batch_size, 1,
            params, thread.MC->get_alphai(), &protocol.P);
    triple_generator->multi_threaded = false;
    this->inputs.resize(thread.P->num_players());
    // init_real(protocol.P);

    P = protocol.get_player();
    std::cout << "Init player number: " << P->my_num() << std::endl;
    MC = protocol.get_mc();
    set_mc(MC);
}

template<class T>
void RmfeSharePrep<T>::set_mc(typename T::MAC_Check* MC) {
    revealed_key = reveal(P, MC->get_alphai());
}

// template<class T>
// void RmfeSharePrep<T>::get_input_no_count(T& r_share, typename T::open_type& r , int player) {
//     typedef typename T::open_type U;
//     typedef typename T::mac_type W;

//     r.randomize(prng);
    
//     array<U, 2> value_shares;
//     array<W, 2> mac_shares;
//     value_shares[0].randomize(prng);
//     value_shares[1] = r - value_shares[0];
//     mac_shares[0].randomize(prng);
//     mac_shares[1] = r * revealed_key - mac_shares[0];

//     // `player` denotes the input's owner, but here we should use the share's owner
//     r_share = {value_shares[P->my_num()], mac_shares[P->my_num()]};
// }

template<class T>
void RmfeSharePrep<T>::get_three_no_count(Dtype dtype, T& a, T& b, T& c) {
    typedef typename T::raw_type V;
    typedef typename T::open_type U;
    typedef typename T::mac_type W;
    array<array<V, 3>, 2> raw_triples;
    raw_triples[0][0].randomize(prng, T::default_length);
    raw_triples[0][1].randomize(prng, T::default_length);
    raw_triples[0][2].randomize(prng, T::default_length);
    raw_triples[1][0].randomize(prng, T::default_length);
    raw_triples[1][1].randomize(prng, T::default_length);
    raw_triples[1][2] = 
        V(raw_triples[0][0] + raw_triples[1][0]) * 
        V(raw_triples[0][1] + raw_triples[1][1]) - 
        raw_triples[0][2];

    array<array<U, 3>, 2> plain_triples;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            plain_triples[i][j] = U(raw_triples[i][j]);
    
    array<array<W, 3>, 2> mac_triples;
    mac_triples[0][0].randomize(prng);
    mac_triples[0][1].randomize(prng);
    mac_triples[0][2].randomize(prng);
    mac_triples[1][0] = U(plain_triples[0][0] + plain_triples[1][0]) * revealed_key - mac_triples[0][0];
    mac_triples[1][1] = U(plain_triples[0][1] + plain_triples[1][1]) * revealed_key - mac_triples[0][1];
    mac_triples[1][2] = U(plain_triples[0][2] + plain_triples[1][2]) * revealed_key - mac_triples[0][2];

    int i = P->my_num();
    a = {plain_triples[i][0], mac_triples[i][0]};
    b = {plain_triples[i][1], mac_triples[i][1]};
    c = {plain_triples[i][2], mac_triples[i][2]};
}

template<class T>
void RmfeSharePrep<T>::buffer_inputs(int player) {
    auto& inputs = this->inputs;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    for (auto& x : triple_generator->inputs)
        inputs.at(player).push_back(x);
}

}

#endif
