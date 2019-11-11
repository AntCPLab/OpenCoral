/*
 * ShamirInput.h
 *
 */

#ifndef PROTOCOLS_SHAMIRINPUT_H_
#define PROTOCOLS_SHAMIRINPUT_H_

#include "Processor/Input.h"
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
    void finalize_other(int player, T& target, octetStream& o, int n_bits = -1);
};

template<class T>
class ShamirInput : public IndividualInput<T>
{
    friend class Shamir<typename T::clear>;

    static vector<vector<typename T::clear>> vandermonde;

    SeededPRNG secure_prng;

    vector<typename T::Scalar> randomness;

public:
    static const vector<vector<typename T::clear>>& get_vandermonde(size_t t,
            size_t n);

    ShamirInput(SubProcessor<T>& proc, ShamirMC<T>& MC) :
            IndividualInput<T>(proc)
    {
        (void) MC;
    }

    ShamirInput(SubProcessor<T>* proc, Player& P) :
            IndividualInput<T>(proc, P)
    {
    }

    void add_mine(const typename T::clear& input, int n_bits = -1);
};

#endif /* PROTOCOLS_SHAMIRINPUT_H_ */
