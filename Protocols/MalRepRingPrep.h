/*
 * MalRepRingPrep.h
 *
 */

#ifndef PROTOCOLS_MALREPRINGPREP_H_
#define PROTOCOLS_MALREPRINGPREP_H_

#include "Protocols/ReplicatedPrep.h"

template<class T>
class MalRepRingPrep : public MaliciousRingPrep<T>
{
public:
    MalRepRingPrep(SubProcessor<T>* proc, DataPositions& usage);

    void buffer_triples();
    void simple_buffer_triples();
    void shuffle_buffer_triples();

    void buffer_squares();

    void buffer_inputs(int player);
};

// extra class to avoid recursion
template<class T>
class MalRepRingPrepWithBits : public virtual MalRepRingPrep<T>
{
public:
    MalRepRingPrepWithBits(SubProcessor<T>* proc, DataPositions& usage);

    void buffer_bits();
};

#endif /* PROTOCOLS_MALREPRINGPREP_H_ */
