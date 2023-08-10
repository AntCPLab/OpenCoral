/*
 * RmfeSharePrep.cpp
 *
 */

#ifndef GC_RMFESHARE_PREP_HPP_
#define GC_RMFESHARE_PREP_HPP_

#include "RmfeSharePrep.h"

#include "Protocols/TinyOt2Rmfe.h"
#include "PersonalPrep.hpp"
#include "Tools/debug.h"
#include "TinyOT/tinyotprep.hpp"
#include "TinyOT/tinyotshare.hpp"

namespace GC
{

template<class T>
RmfeSharePrep<T>::RmfeSharePrep(DataPositions& usage, int input_player) :
        PersonalPrep<T>(usage, input_player),
        triple_generator(0),
        tinyot2rmfe(0)
        // spdz2k2rmfe(0)
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
    if (tinyot2rmfe) {
        delete tinyot2rmfe;
    }
    // if (spdz2k2rmfe) {
    //     delete spdz2k2rmfe;
    // }
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

    Player* player = protocol.get_player();

    int tinyot_batch_size = triple_generator->nTriplesPerLoop * T::default_length;
    tinyot2rmfe = new RmfeShareConverter<TinyOTShare>(*player);
    tinyot2rmfe->get_src_prep()->set_batch_size(tinyot_batch_size);

    // spdz2k2rmfe = new RmfeShareConverter<Spdz2kBShare>(*player);

    MC = protocol.get_mc();
}

template<class T>
void RmfeSharePrep<T>::set_mc(typename T::MAC_Check* MC) {
    revealed_key = reveal(P, MC->get_alphai());
}

template<class T>
void RmfeSharePrep<T>::buffer_inputs(int player) {
    auto& inputs = this->inputs;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    for (auto& x : triple_generator->inputs)
        inputs.at(player).push_back(x);
}

template<class T>
void RmfeSharePrep<T>::buffer_triples() {
    print_general("enter buffer triples");

    auto& nTriplesPerLoop = triple_generator->nTriplesPerLoop;
    auto tinyot_prep = tinyot2rmfe->get_src_prep();

    int l = T::default_length;
    // triple storage arranged as: n x 3 x l
    vector<TinyOTShare> tinyot_shares(nTriplesPerLoop * 3 * l);
    // triple storage arranged as: n x 3
    vector<T> rmfe_shares;

    // Generate tinyot triples
    for(int i = 0; i < nTriplesPerLoop; i++) {
        for(int j = 0; j < l; j++) {
            tinyot_prep->get_tinyot_triple(
                tinyot_shares[i * 3 * l + j].MAC, tinyot_shares[i * 3 + j].KEY,
                tinyot_shares[i * 3 * l + l + j].MAC, tinyot_shares[i * 3 * l + l + j].KEY,
                tinyot_shares[i * 3 * l + 2 * l + j].MAC, tinyot_shares[i * 3 * l + 2 * l + j].KEY);
        }
    }
    print_general("gen tinyot triples");

    // Convert tinyot shares to rmfe shares
    tinyot2rmfe->convert(rmfe_shares, tinyot_shares);
    print_general("convert to rmfe triples");
    assert((int)rmfe_shares.size() == nTriplesPerLoop * 3);

    for(int i = 0; i < nTriplesPerLoop; i++) {
        this->triples.push_back({{rmfe_shares[i*3], rmfe_shares[i*3 + 1], rmfe_shares[i*3 + 2]}});
    }
    print_general("store rmfe triples");
}


#ifdef INSECURE_RMFE_PREP
template<class T>
void RmfeSharePrep<T>::get_input_no_count(T& r_share, typename T::open_type& r , int player) {
    typedef typename T::open_type U;
    typedef typename T::mac_type W;

    r.randomize(prng);
    
    array<U, 2> value_shares;
    array<W, 2> mac_shares;
    value_shares[0].randomize(prng);
    value_shares[1] = r - value_shares[0];
    mac_shares[0].randomize(prng);
    mac_shares[1] = r * revealed_key - mac_shares[0];

    // `player` denotes the input's owner, but here we should use the share's owner
    r_share = {value_shares[P->my_num()], mac_shares[P->my_num()]};
}

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
#endif

}

#endif
