/*
 * MascotPrep.cpp
 *
 */

#ifndef PROTOCOLS_MASCOTPREP_HPP_
#define PROTOCOLS_MASCOTPREP_HPP_

#include "MascotPrep.h"
#include "Processor/Processor.h"
#include "Processor/BaseMachine.h"
#include "OT/OTTripleSetup.h"
#include "OT/Triple.hpp"
#include "OT/NPartyTripleGenerator.hpp"
#include "Protocols/ShuffleSacrifice.hpp"
#include "Protocols/Spdz2kPrep.hpp"
#include "Protocols/ReplicatedPrep.hpp"

template<class T>
OTPrep<T>::OTPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage), BitPrep<T>(proc, usage),
        RingPrep<T>(proc, usage), triple_generator(0)
{
}

template<class T>
OTPrep<T>::~OTPrep()
{
    if (triple_generator)
        delete triple_generator;
}

template<class T>
void OTPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    RingPrep<T>::set_protocol(protocol);
    SubProcessor<T>* proc = this->proc;
    assert(proc != 0);
    triple_generator = new typename T::TripleGenerator(
            BaseMachine::s().fresh_ot_setup(),
            proc->P.N, -1,
            OnlineOptions::singleton.batch_size, 1,
            params, proc->MC.get_alphai(), &proc->P);
    triple_generator->multi_threaded = false;
}

template<class T>
void MascotPrep<T>::buffer_triples()
{
#ifdef INSECURE
#ifdef FAKE_MASCOT_TRIPLES
    this->triples.resize(this->triples.size() + OnlineOptions::singleton.batch_size);
    return;
#endif
#endif

    auto& params = this->params;
    auto& triple_generator = this->triple_generator;
    params.generateBits = false;
    triple_generator->generate();
    triple_generator->unlock();
    assert(triple_generator->uncheckedTriples.size() != 0);
    for (auto& triple : triple_generator->uncheckedTriples)
        this->triples.push_back(
        {{ triple.a[0], triple.b, triple.c[0] }});
}

template<class T>
void MascotFieldPrep<T>::buffer_inverses()
{
    assert(this->proc != 0);
    ::buffer_inverses(this->inverses, *this, this->proc->MC, this->proc->P);
}

template<class T>
void MascotFieldPrep<T>::buffer_bits()
{
    this->params.generateBits = true;
    auto& triple_generator = this->triple_generator;
    triple_generator->generate();
    triple_generator->unlock();
    assert(triple_generator->bits.size() != 0);
    for (auto& bit : triple_generator->bits)
        this->bits.push_back(bit);
}

template<class T>
void MascotPrep<T>::buffer_dabits(ThreadQueues* queues)
{
    assert(this->proc != 0);
    DabitSacrifice<T> dabit_sacrifice;
    vector<dabit<T>> check_dabits;
#ifdef VERBOSE_DABIT
    cerr << "Generate daBits to sacrifice" << endl;
#endif
    this->buffer_dabits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(), queues);
    dabit_sacrifice.sacrifice_and_check_bits(this->dabits, check_dabits,
            *this->proc, queues);
}

template<class T>
void MascotPrep<T>::buffer_inputs(int player)
{
    auto& triple_generator = this->triple_generator;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    if (this->inputs.size() <= (size_t)player)
        this->inputs.resize(player + 1);
    for (auto& input : triple_generator->inputs)
        this->inputs[player].push_back(input);
}

template<class T>
T MascotPrep<T>::get_random()
{
    assert(this->proc);
    return BufferPrep<T>::get_random_from_inputs(this->proc->P.num_players());
}

template<class T>
T BufferPrep<T>::get_random_from_inputs(int nplayers)
{
    T res;
    for (int j = 0; j < nplayers; j++)
    {
        T tmp;
        typename T::open_type _;
        this->get_input_no_count(tmp, _, j);
        res += tmp;
    }
    return res;
}

template<class T>
size_t OTPrep<T>::data_sent()
{
    size_t res = RingPrep<T>::data_sent();
    if (triple_generator)
        res += triple_generator->data_sent();
    return res;
}

template<class T>
NamedCommStats OTPrep<T>::comm_stats()
{
    if (triple_generator)
        return triple_generator->comm_stats();
    else
        return {};
}

#endif
