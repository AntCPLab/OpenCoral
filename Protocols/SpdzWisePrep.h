/*
 * SpdzWisePrep.h
 *
 */

#ifndef PROTOCOLS_SPDZWISEPREP_H_
#define PROTOCOLS_SPDZWISEPREP_H_

#include "ReplicatedPrep.h"

template<class T>
class SpdzWisePrep : public MaliciousRingPrep<T>
{
    typedef MaliciousRingPrep<T> super;

    void buffer_triples();
    void buffer_bits();
    void buffer_inverses();

    void buffer_inputs(int player);

public:
    SpdzWisePrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage),
        BitPrep<T>(proc, usage), RingPrep<T>(proc, usage),
        MaliciousRingPrep<T>(proc, usage)
    {
    }
};

#endif /* PROTOCOLS_SPDZWISEPREP_H_ */
