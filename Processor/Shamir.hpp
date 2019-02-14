/*
 * Shamir.cpp
 *
 */

#include "Shamir.h"
#include "ShamirInput.h"
#include "Machines/ShamirMachine.h"

template<class U>
U Shamir<U>::get_rec_factor(int i, int n)
{
    U res = 1;
    for (int j = 0; j < n; j++)
        if (i != j)
            res *= U(j + 1) / (U(j + 1) - U(i + 1));
    return res;
}

template<class U>
Shamir<U>::Shamir(Player& P) : resharing(0), P(P)
{
    threshold = ShamirMachine::s().threshold;
    n_mul_players = 2 * threshold + 1;
}

template<class U>
Shamir<U>::~Shamir()
{
    if (resharing != 0)
        delete resharing;
}

template<class U>
int Shamir<U>::get_n_relevant_players()
{
    return ShamirMachine::s().threshold + 1;
}

template<class U>
void Shamir<U>::reset()
{
    os.clear();
    os.resize(P.num_players());

    if (resharing == 0)
    {
        resharing = new ShamirInput<T>(0, P);
    }

    for (int i = 0; i < P.num_players(); i++)
        resharing->reset(i);
}

template<class U>
void Shamir<U>::init_mul(SubProcessor<T>* proc)
{
    (void) proc;
    reset();
    if (rec_factor == 0 and P.my_num() < n_mul_players)
        rec_factor = get_rec_factor(P.my_num(), n_mul_players);
}

template<class U>
U Shamir<U>::prepare_mul(const T& x, const T& y)
{
    auto add_share = x * y * rec_factor;
    if (P.my_num() < n_mul_players)
        resharing->add_mine(add_share);
    return add_share;
}

template<class U>
void Shamir<U>::exchange()
{
    if (P.my_num() < n_mul_players)
        resharing->send_mine();
    for (int i = 0; i < n_mul_players; i++)
        if (i != P.my_num())
            P.receive_player(i, os[i], true);
}

template<class U>
ShamirShare<U> Shamir<U>::finalize_mul()
{
    return finalize(n_mul_players);
}

template<class U>
ShamirShare<U> Shamir<U>::finalize(int n_relevant_players)
{
    ShamirShare<U> res = U(0);
    if (P.my_num() < n_relevant_players)
        res = resharing->finalize_mine();
    for (int i = 0; i < n_relevant_players; i++)
        if (i != P.my_num())
        {
            T tmp;
            resharing->finalize_other(i, tmp, os[i]);
            res += tmp;
        }
    return res;
}

template<class U>
ShamirShare<U> Shamir<U>::get_random()
{
    if (random.empty())
        buffer_random();
    auto res = random.back();
    random.pop_back();
    return res;
}

template<class U>
void Shamir<U>::buffer_random()
{
    Shamir<U> shamir(P);
    shamir.reset();
    int buffer_size = 1000;
    if (P.my_num() <= threshold)
        for (int i = 0; i < buffer_size; i++)
            shamir.resharing->add_mine(secure_prng.get<U>());
    shamir.exchange();
    for (int i = 0; i < buffer_size; i++)
        random.push_back(shamir.finalize(threshold + 1));
}
