/*
 * DealerPrep.h
 *
 */

#ifndef PROTOCOLS_DEALERPREP_H_
#define PROTOCOLS_DEALERPREP_H_

#include "ReplicatedPrep.h"

template<class T>
class DealerPrep : virtual public BitPrep<T>
{
    template<int = 0>
    void buffer_edabits(int n_bits, true_type);
    template<int = 0>
    void buffer_edabits(int n_bits, false_type);

public:
    DealerPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage)
    {
    }

    void buffer_triples();
    void buffer_bits();

    void buffer_dabits(ThreadQueues* = 0);
    void buffer_edabits(int n_bits, ThreadQueues*);
    void buffer_sedabits(int n_bits, ThreadQueues*);
};

#endif /* PROTOCOLS_DEALERPREP_H_ */
