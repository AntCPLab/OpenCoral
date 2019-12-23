/*
 * MaliciousRingPrep.hpp
 *
 */

#ifndef PROTOCOLS_MALICIOUSRINGPREP_HPP_
#define PROTOCOLS_MALICIOUSRINGPREP_HPP_

#include "ReplicatedPrep.h"

#include "ShuffleSacrifice.hpp"

template<class T>
void MaliciousRingPrep<T>::buffer_dabits()
{
    assert(this->proc != 0);
    vector<dabit<T>> check_dabits;
    ShuffleSacrifice<T> shuffle_sacrifice;
    this->buffer_dabits_without_check(check_dabits,
            shuffle_sacrifice.minimum_n_inputs());
    shuffle_sacrifice.dabit_sacrifice(this->dabits, check_dabits, *this->proc);
}

#endif /* PROTOCOLS_MALICIOUSRINGPREP_HPP_ */
