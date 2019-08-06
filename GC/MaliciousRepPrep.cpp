/*
 * MaliciousRepPrep.cpp
 *
 */

#include "MaliciousRepPrep.h"
#include "MaliciousRepThread.h"
#include "Processor/OnlineOptions.h"

#include "Protocols/MalRepRingPrep.hpp"
#include "Protocols/ReplicatedPrep.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/MAC_Check_Base.hpp"

namespace GC
{

MaliciousRepPrep::MaliciousRepPrep(DataPositions& usage) :
        BufferPrep<MaliciousRepSecret>(usage), protocol(0)
{
}

MaliciousRepPrep::~MaliciousRepPrep()
{
    if (protocol)
        delete protocol;
}

void MaliciousRepPrep::set_protocol(MaliciousRepSecret::Protocol& protocol)
{
    this->protocol = new ReplicatedBase(protocol.P);
}

void MaliciousRepPrep::buffer_triples()
{
    assert(protocol != 0);
    auto MC = MaliciousRepThread::s().new_mc();
    shuffle_triple_generation(triples, protocol->P, *MC, 64);
    delete MC;
}

void MaliciousRepPrep::buffer_bits()
{
    assert(this->protocol != 0);
    for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
    {
        this->bits.push_back({});
        for (int j = 0; j < 2; j++)
            this->bits.back()[j] = this->protocol->shared_prngs[j].get_bit();
    }
}

} /* namespace GC */
