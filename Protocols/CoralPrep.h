/*
 * CoralPrep.h
 *
 */

#ifndef PROTOCOLS_CoralPREP_H_
#define PROTOCOLS_CoralPREP_H_

#include "Spdz2kPrep.h"

template<int, int> class CoralShare;

/**
 * Coral preprocessing
 */
template<class T>
class CoralPrep : public virtual Spdz2kPrep<T>
{
    // typedef typename T::bit_prep_type BitShare;
    // DataPositions bit_pos;
    // MascotTriplePrep<BitShare>* bit_prep;
    // SubProcessor<BitShare>* bit_proc;
    // typename BitShare::MAC_Check* bit_MC;

public:
    CoralPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~CoralPrep();

    void buffer_dabits(ThreadQueues* queues);
};

#endif /* PROTOCOLS_SPDZ2KPREP_H_ */
