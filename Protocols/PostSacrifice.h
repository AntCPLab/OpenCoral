/*
 * PostSacrifice.h
 *
 */

#ifndef PROTOCOLS_POSTSACRIFICE_H_
#define PROTOCOLS_POSTSACRIFICE_H_

#include "Protocols/Replicated.h"

template<class T>
class PostSacrifice : public ProtocolBase<T>
{
    typename T::Honest::Protocol internal;

    vector<array<T, 2>> operands;
    vector<T> results;

    void check();

public:
    Player& P;

    PostSacrifice(Player& P);
    ~PostSacrifice();

    void init_mul(SubProcessor<T>* proc);
    typename T::clear prepare_mul(const T& x, const T& y);
    void exchange() { internal.exchange(); }
    T finalize_mul();

    int get_n_relevant_players() { return internal.get_n_relevant_players(); }
};

#endif /* PROTOCOLS_POSTSACRIFICE_H_ */
