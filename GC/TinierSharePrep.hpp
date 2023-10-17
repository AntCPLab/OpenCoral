/*
 * TinierSharePrep.cpp
 *
 */

#ifndef GC_TINIERSHARE_PREP_HPP_
#define GC_TINIERSHARE_PREP_HPP_

#include "TinierSharePrep.h"

#include "PersonalPrep.hpp"

namespace GC
{

template<class T>
TinierSharePrep<T>::TinierSharePrep(DataPositions& usage, int input_player) :
        PersonalPrep<T>(usage, input_player), triple_generator(0),
        real_triple_generator(0)
{
}

template<class T>
TinierSharePrep<T>::TinierSharePrep(SubProcessor<T>*, DataPositions& usage) :
        TinierSharePrep(usage)
{
}

template<class T>
TinierSharePrep<T>::~TinierSharePrep()
{
    if (triple_generator)
        delete triple_generator;
    if (real_triple_generator)
        delete real_triple_generator;
#ifdef SPDZ2K_SP
    if (tinyot2spdz2k)
        delete tinyot2spdz2k;
#endif
}

template<class T>
void TinierSharePrep<T>::set_protocol(typename T::Protocol& protocol)
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

#ifdef SPDZ2K_SP
    int tinyot_batch_size = triple_generator->nTriplesPerLoop * T::default_length;
    tinyot2spdz2k = new GeneralShareConverter<TinyOTShare, T>(protocol.P);
    tinyot2spdz2k->get_src_prep()->set_batch_size(tinyot_batch_size);
#endif
}

template<class T>
void TinierSharePrep<T>::buffer_triples()
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Tinier triples", ShareThread<secret_type>::s().P->total_comm().sent);
#endif

    if (this->input_player != this->SECURE)
    {
        this->buffer_personal_triples();
    }
    else {
#ifdef SPDZ2K_SP
        buffer_secret_triples_spdz2ksp();
#else
        buffer_secret_triples();
#endif
    }

#ifdef DETAIL_BENCHMARK
    perf.stop(ShareThread<secret_type>::s().P->total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

template<class T>
void TinierSharePrep<T>::buffer_inputs(int player)
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Tinier inputs", ShareThread<secret_type>::s().P->total_comm().sent);
#endif

    auto& inputs = this->inputs;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    for (auto& x : triple_generator->inputs)
        inputs.at(player).push_back(x);

#ifdef DETAIL_BENCHMARK
    perf.stop(ShareThread<secret_type>::s().P->total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

template<class T>
void GC::TinierSharePrep<T>::buffer_bits()
{
#ifdef DETAIL_BENCHMARK
    ThreadPerformance perf("Tinier bits", ShareThread<secret_type>::s().P->total_comm().sent);
#endif

    auto& thread = ShareThread<secret_type>::s();
    this->bits.push_back(
            BufferPrep<T>::get_random_from_inputs(thread.P->num_players()));

#ifdef DETAIL_BENCHMARK
    perf.stop(ShareThread<secret_type>::s().P->total_comm().sent);
    GlobalPerformance::s().add(perf);
#endif
}

}

#endif
