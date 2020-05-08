/*
 * BrainPrep.h
 *
 */

#ifndef PROTOCOLS_BRAINPREP_H_
#define PROTOCOLS_BRAINPREP_H_

#include "ReplicatedPrep.h"
#include "Protocols/BrainShare.h"

template<class T>
class BrainPrep : public MaliciousRingPrep<T>
{
public:
    BrainPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage),
            MaliciousRingPrep<T>(proc, usage)
    {
    }

    void buffer_triples();
    void buffer_inputs(int player);
};

#endif /* PROTOCOLS_BRAINPREP_H_ */
