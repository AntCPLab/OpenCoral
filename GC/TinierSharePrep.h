/*
 * TinierSharePrep.h
 *
 */

#ifndef GC_TINIERSHAREPREP_H_
#define GC_TINIERSHAREPREP_H_

#include "Protocols/ReplicatedPrep.h"
#include "OT/NPartyTripleGenerator.h"
#include "ShareThread.h"

namespace GC
{

template<class T>
class TinierSharePrep : public BufferPrep<T>
{
    typename T::TripleGenerator* triple_generator;
    MascotParams params;

    void buffer_triples() { throw not_implemented(); }
    void buffer_squares() { throw not_implemented(); }
    void buffer_bits() { throw not_implemented(); }
    void buffer_inverses() { throw not_implemented(); }

    void buffer_inputs(int player);

public:
    TinierSharePrep(DataPositions& usage);
    ~TinierSharePrep();

    void set_protocol(typename T::Protocol& protocol);

    size_t data_sent();
};

}

#endif /* GC_TINIERSHAREPREP_H_ */
