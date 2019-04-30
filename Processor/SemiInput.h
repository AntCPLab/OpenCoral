/*
 * SemiInput.h
 *
 */

#ifndef PROCESSOR_SEMIINPUT_H_
#define PROCESSOR_SEMIINPUT_H_

#include "ShamirInput.h"

template<class T> class SemiMC;

template<class T>
class SemiInput : public IndividualInput<T>
{
    SeededPRNG secure_prng;

public:
    SemiInput(SubProcessor<T>& proc, SemiMC<T>& MC) :
            IndividualInput<T>(proc)
    {
        (void) MC;
    }

    SemiInput(SubProcessor<T>* proc, Player& P) :
            IndividualInput<T>(proc, P)
    {
    }

    void add_mine(const typename T::clear& input);
};

#endif /* PROCESSOR_SEMIINPUT_H_ */
