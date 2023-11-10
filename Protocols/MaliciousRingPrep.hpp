/*
 * MaliciousRingPrep.hpp
 *
 */

#ifndef PROTOCOLS_MALICIOUSRINGPREP_HPP_
#define PROTOCOLS_MALICIOUSRINGPREP_HPP_

#include "ReplicatedPrep.h"

#include "DabitSacrifice.hpp"
#include "Spdz2kPrep.hpp"
#include "ShuffleSacrifice.hpp"

template<class T>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues* queues)
{
    buffer_dabits<0>(queues, T::clear::characteristic_two,
            T::clear::prime_field);
}

template<class T>
template<int>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues*, true_type, false_type)
{
    throw runtime_error("only implemented for integer-like domains");
}

template<class T>
template<int>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues* queues, false_type, false_type)
{
    assert(this->proc != 0);
    if (T::bit_type::tight_packed) {
        size_t prev_size = this->dabitpacks.size();
        vector<dabitpack<T>> check_dabits;
        this->buffer_dabits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(this->buffer_size, T::bit_type::default_length), queues);
        dabit_sacrifice.sacrifice_and_check_bits(this->dabitpacks, check_dabits, *this->proc, queues);

        print_general("Generate dabitpack", this->dabitpacks.size() - prev_size);
    }
    else {
        size_t prev_size = this->dabits.size();
        vector<dabit<T>> check_dabits;
        this->buffer_dabits_without_check(check_dabits,
                dabit_sacrifice.minimum_n_inputs(this->buffer_size), queues);
        dabit_sacrifice.sacrifice_and_check_bits(this->dabits, check_dabits,
                *this->proc, queues);

        print_general("Generate dabit", this->dabits.size() - prev_size);
    }
}

template<class T>
template<int>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues* queues, false_type, true_type)
{
    // [zico] This branch is because the `daBit check` protocol in Fig.16 of edabits paper
    // requires the integer domain to have bit length > statiscial security parameter.
    if (T::clear::length() >= 60)
        buffer_dabits<0>(queues, false_type(), false_type());
    else
    {
        // [zico] TODO: This branch has NOT been updated to deal with `dabitpack`
        // because our experiments never use primes of bit length smaller than 60
        assert(this->proc != 0);
        vector<dabit<T>> check_dabits;
        DabitShuffleSacrifice<T> shuffle_sacrifice;
        this->buffer_dabits_without_check(check_dabits,
                shuffle_sacrifice.minimum_n_inputs(), queues);
        shuffle_sacrifice.dabit_sacrifice(this->dabits, check_dabits, *this->proc,
                queues);
    }
}

#endif /* PROTOCOLS_MALICIOUSRINGPREP_HPP_ */
