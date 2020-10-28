/*
 * RepRingOnlyEdabitPrep.h
 *
 */

#ifndef PROTOCOLS_REPRINGONLYEDABITPREP_H_
#define PROTOCOLS_REPRINGONLYEDABITPREP_H_

#include "ReplicatedPrep.h"

template<class T>
class RepRingOnlyEdabitPrep : public virtual BufferPrep<T>
{
protected:
    void buffer_edabits(int n_bits, ThreadQueues*);

public:
    RepRingOnlyEdabitPrep(SubProcessor<T>*, DataPositions& usage) :
            BufferPrep<T>(usage)
    {
    }
};

#endif /* PROTOCOLS_REPRINGONLYEDABITPREP_H_ */
