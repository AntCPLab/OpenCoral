/*
 * MaliciousRingPrep.hpp
 *
 */

#ifndef PROTOCOLS_MALICIOUSRINGPREP_HPP_
#define PROTOCOLS_MALICIOUSRINGPREP_HPP_

#include "ReplicatedPrep.h"

#include "DabitSacrifice.hpp"
#include "Spdz2kPrep.hpp"

template<class T>
void MaliciousRingPrep<T>::buffer_dabits(ThreadQueues* queues)
{
    assert(this->proc != 0);
    vector<dabit<T>> check_dabits;
    DabitSacrifice<T> dabit_sacrifice;
    this->buffer_dabits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(), queues);
    dabit_sacrifice.sacrifice_and_check_bits(this->dabits, check_dabits,
            *this->proc, queues);
}

#endif /* PROTOCOLS_MALICIOUSRINGPREP_HPP_ */
