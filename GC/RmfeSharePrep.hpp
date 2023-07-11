/*
 * RmfeSharePrep.cpp
 *
 */

#ifndef GC_RMFESHARE_PREP_HPP_
#define GC_RMFESHARE_PREP_HPP_

#include "RmfeSharePrep.h"

#include "PersonalPrep.hpp"

namespace GC
{

template<class T>
RmfeSharePrep<T>::RmfeSharePrep(DataPositions& usage, int input_player) :
        PersonalPrep<T>(usage, input_player), triple_generator(0),
        real_triple_generator(0)
{
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
    if (real_triple_generator)
        delete real_triple_generator;
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
    init_real(protocol.P);
}

template<class T>
void RmfeSharePrep<T>::buffer_triples()
{
    if (this->input_player != this->SECURE)
    {
        this->buffer_personal_triples();
        return;
    }
    else
        buffer_secret_triples();
}

template<class T>
void RmfeSharePrep<T>::buffer_inputs(int player)
{
    auto& inputs = this->inputs;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    for (auto& x : triple_generator->inputs)
        inputs.at(player).push_back(x);
}

template<class T>
void GC::RmfeSharePrep<T>::buffer_bits()
{
    auto& thread = ShareThread<secret_type>::s();
    this->bits.push_back(
            BufferPrep<T>::get_random_from_inputs(thread.P->num_players()));
}

}

template<class T>
void GC::RmfeSharePrep<T>::get_input_no_count(T& r_share, typename T::open_type& r , int player) {
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

#endif
