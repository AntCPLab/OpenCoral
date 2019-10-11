/*
 * MaliciousRepPrep.cpp
 *
 */

#include "RepPrep.h"
#include "ShareThread.h"
#include "Processor/OnlineOptions.h"

#include "Protocols/MalRepRingPrep.hpp"
#include "Protocols/ReplicatedPrep.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/MAC_Check_Base.hpp"

namespace GC
{

template<class T>
RepPrep<T>::RepPrep(DataPositions& usage, Thread<T>& thread) :
        BufferPrep<T>(usage), protocol(0)
{
    (void) thread;
}

template<class T>
RepPrep<T>::~RepPrep()
{
    if (protocol)
        delete protocol;
}

template<class T>
void RepPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    this->protocol = new ReplicatedBase(protocol.P);
}

template<class T>
void RepPrep<T>::buffer_triples()
{
    assert(protocol != 0);
    auto MC = ShareThread<T>::s().new_mc();
    shuffle_triple_generation(this->triples, protocol->P, *MC, 64);
    delete MC;
}

template<class T>
void RepPrep<T>::buffer_bits()
{
    assert(this->protocol != 0);
    assert(this->protocol->P.num_players() == 3);
    for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
    {
        this->bits.push_back({});
        for (int j = 0; j < 2; j++)
            this->bits.back()[j] = this->protocol->shared_prngs[j].get_bit();
    }
}

} /* namespace GC */
