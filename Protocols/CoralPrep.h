/*
 * CoralPrep.h
 *
 */

#ifndef PROTOCOLS_CoralPREP_H_
#define PROTOCOLS_CoralPREP_H_

#include "CoralShare.h"
#include "Spdz2kPrep.h"
#include "GC/Spdz2kBShare.h"


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

    RmfeShareConverter<GC::Spdz2kBShare<T::s>>* spdz2k2rmfe;

public:
    CoralPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~CoralPrep();

    void set_protocol(typename T::Protocol& protocol);

    void get_dabit(T& a, typename T::bit_type& b);
    void buffer_dabits(ThreadQueues* queues = 0);
};

#endif /* PROTOCOLS_SPDZ2KPREP_H_ */
