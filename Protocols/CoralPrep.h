/*
 * CoralPrep.h
 *
 */

#ifndef PROTOCOLS_CORALPREP_H_
#define PROTOCOLS_CORALPREP_H_

#include "CoralShare.h"
#include "Spdz2kPrep.h"
#include "GC/Spdz2kBShare.h"


/**
 * Coral preprocessing
 */
template<class T>
class CoralPrep : public virtual Spdz2kPrep<T>
{

public:
    CoralPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~CoralPrep();

    void set_protocol(typename T::Protocol& protocol);

    void get_dabit(T& a, typename T::bit_type& b);
    void buffer_dabits(ThreadQueues* queues = 0);
};

#endif /* PROTOCOLS_CORALPREP_H_ */
