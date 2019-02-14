/*
 * Shamir.h
 *
 */

#ifndef PROCESSOR_SHAMIR_H_
#define PROCESSOR_SHAMIR_H_

#include <vector>
using namespace std;

#include "Replicated.h"

template<class T> class SubProcessor;
template<class T> class ShamirMC;
template<class T> class ShamirShare;
template<class T> class ShamirInput;

class Player;

template<class U>
class Shamir : public ProtocolBase<ShamirShare<U>>
{
    typedef ShamirShare<U> T;

    vector<octetStream> os;
    vector<U> reconstruction;
    U rec_factor;
    ShamirInput<T>* resharing;

    SeededPRNG secure_prng;

    vector<T> random;

    void buffer_random();

    int threshold;
    int n_mul_players;

public:
    Player& P;

    static U get_rec_factor(int i, int n);

    Shamir(Player& P);
    ~Shamir();

    int get_n_relevant_players();

    void reset();

    void init_mul(SubProcessor<T>* proc);
    U prepare_mul(const T& x, const T& y);
    void exchange();
    T finalize_mul();

    T finalize(int n_input_players);

    T get_random();
};

#endif /* PROCESSOR_SHAMIR_H_ */
