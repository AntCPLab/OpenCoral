/*
 * ShamirInput.h
 *
 */

#ifndef PROCESSOR_SHAMIRINPUT_H_
#define PROCESSOR_SHAMIRINPUT_H_

#include "Input.h"
#include "Shamir.h"
#include "ReplicatedInput.h"

template<class T>
class ShamirInput : public PrepLessInput<T>
{
    Player& P;
    vector<octetStream> os;
    vector<vector<typename T::clear>> vandermonde;
    SeededPRNG secure_prng;

    vector<T> randomness;

public:
    ShamirInput(SubProcessor<T>& proc) :
            PrepLessInput<T>(&proc), P(proc.P)
    {
    }

    ShamirInput(SubProcessor<T>& proc, ShamirMC<T>& MC) :
            ShamirInput(proc)
    {
        (void) MC;
    }

    ShamirInput(SubProcessor<T>* proc, Player& P) :
            PrepLessInput<T>(proc), P(P)
    {
    }

    void reset(int player);
    void add_mine(const typename T::clear& input);
    void add_other(int player);
    void send_mine();
    void finalize_other(int player, T& target, octetStream& o);
};

#endif /* PROCESSOR_SHAMIRINPUT_H_ */
