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
class IndividualInput : public PrepLessInput<T>
{
protected:
    Player& P;
    vector<octetStream> os;

public:
    IndividualInput(SubProcessor<T>* proc, Player& P) :
            PrepLessInput<T>(proc), P(P)
    {
    }
    IndividualInput(SubProcessor<T>& proc) :
            PrepLessInput<T>(&proc), P(proc.P)
    {
    }

    void reset(int player);
    void add_other(int player);
    void send_mine();
    void finalize_other(int player, T& target, octetStream& o);
};

template<class T>
class ShamirInput : public IndividualInput<T>
{
    friend class Shamir<typename T::clear>;

    vector<vector<typename T::clear>> vandermonde;
    SeededPRNG secure_prng;

    vector<T> randomness;

public:
    ShamirInput(SubProcessor<T>& proc, ShamirMC<T>& MC) :
            IndividualInput<T>(proc)
    {
        (void) MC;
    }

    ShamirInput(SubProcessor<T>* proc, Player& P) :
            IndividualInput<T>(proc, P)
    {
    }

    void add_mine(const typename T::clear& input);
};

#endif /* PROCESSOR_SHAMIRINPUT_H_ */
