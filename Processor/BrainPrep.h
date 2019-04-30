/*
 * BrainPrep.h
 *
 */

#ifndef PROCESSOR_BRAINPREP_H_
#define PROCESSOR_BRAINPREP_H_

#include "ReplicatedPrep.h"
#include "Math/BrainShare.h"

template<class T>
class BrainPrep : public RingPrep<T>
{
public:
    BrainPrep(SubProcessor<T>* proc, DataPositions& usage) :
        RingPrep<T>(proc, usage) {}
    void buffer_triples();
};

#endif /* PROCESSOR_BRAINPREP_H_ */
