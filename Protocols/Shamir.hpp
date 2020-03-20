/*
 * Shamir.cpp
 *
 */

#ifndef PROTOCOLS_SHAMIR_HPP_
#define PROTOCOLS_SHAMIR_HPP_

#include "Shamir.h"
#include "ShamirInput.h"
#include "Machines/ShamirMachine.h"
#include "Tools/benchmarking.h"

template<class T>
typename T::open_type::Scalar Shamir<T>::get_rec_factor(int i, int n)
{
    U res = 1;
    for (int j = 0; j < n; j++)
        if (i != j)
            res *= U(j + 1) / (U(j + 1) - U(i + 1));
    return res;
}

template<class T>
Shamir<T>::Shamir(Player& P) : resharing(0), P(P)
{
    if (not P.is_encrypted())
        insecure("unencrypted communication");
    threshold = ShamirMachine::s().threshold;
    n_mul_players = 2 * threshold + 1;
}

template<class T>
Shamir<T>::~Shamir()
{
    if (resharing != 0)
        delete resharing;
}

template<class T>
Shamir<T> Shamir<T>::branch()
{
    return P;
}

template<class T>
int Shamir<T>::get_n_relevant_players()
{
    return ShamirMachine::s().threshold + 1;
}

template<class T>
void Shamir<T>::reset()
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

template<class T>
void Shamir<T>::init_mul(SubProcessor<T>* proc)
{
    (void) proc;
    init_mul();
}

template<class T>
void Shamir<T>::init_mul()
{
    reset();
    if (rec_factor == 0 and P.my_num() < n_mul_players)
        rec_factor = get_rec_factor(P.my_num(), n_mul_players);
}

template<class T>
typename T::clear Shamir<T>::prepare_mul(const T& x, const T& y, int n)
{
    (void) n;
    auto add_share = x * y * rec_factor;
    if (P.my_num() < n_mul_players)
        resharing->add_mine(add_share);
    return {};
}

template<class T>
void Shamir<T>::exchange()
{
    for (int offset = 1; offset < P.num_players(); offset++)
    {
        int receive_from = P.get_player(-offset);
        int send_to = P.get_player(offset);
        bool receive = receive_from < n_mul_players;
        if (P.my_num() < n_mul_players)
        {
            if (receive)
                P.pass_around(resharing->os[send_to], os[receive_from], offset);
            else
                P.send_to(send_to, resharing->os[send_to], true);
        }
        else if (receive)
            P.receive_player(receive_from, os[receive_from], true);
    }
}

template<class T>
void Shamir<T>::start_exchange()
{
    if (P.my_num() < n_mul_players)
        for (int offset = 1; offset < P.num_players(); offset++)
            P.send_relative(offset, resharing->os[P.get_player(offset)]);
}

template<class T>
void Shamir<T>::stop_exchange()
{
    for (int offset = 1; offset < P.num_players(); offset++)
    {
        int receive_from = P.get_player(-offset);
        if (receive_from < n_mul_players)
            P.receive_player(receive_from, os[receive_from], true);
    }
}

template<class T>
T Shamir<T>::finalize_mul(int n)
{
    (void) n;
    return finalize(n_mul_players);
}

template<class T>
T Shamir<T>::finalize(int n_relevant_players)
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

template<class T>
T Shamir<T>::get_random()
{
    if (random.empty())
        buffer_random();
    auto res = random.back();
    random.pop_back();
    return res;
}

template<class T>
void Shamir<T>::buffer_random()
{
    Shamir<T> shamir(P);
    shamir.reset();
    int buffer_size = OnlineOptions::singleton.batch_size;
    if (P.my_num() <= threshold)
        for (int i = 0; i < buffer_size; i++)
            shamir.resharing->add_mine(secure_prng.get<U>());
    shamir.exchange();
    for (int i = 0; i < buffer_size; i++)
        random.push_back(shamir.finalize(threshold + 1));
}

#endif
