/*
 * Shamir.h
 *
 */

#ifndef PROTOCOLS_SHAMIR_H_
#define PROTOCOLS_SHAMIR_H_

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
    static const bool uses_triples = false;

    Player& P;

    static U get_rec_factor(int i, int n);

    Shamir(Player& P);
    ~Shamir();

    Shamir branch();

    int get_n_relevant_players();

    void reset();

    void init_mul();
    void init_mul(SubProcessor<T>* proc);
    U prepare_mul(const T& x, const T& y, int n = -1);

    void exchange();
    void start_exchange();
    void stop_exchange();

    T finalize_mul(int n = -1);

    T finalize(int n_input_players);

    T get_random();
};

#endif /* PROTOCOLS_SHAMIR_H_ */
