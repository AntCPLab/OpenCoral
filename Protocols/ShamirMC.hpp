/*
 * ShamirMC.cpp
 *
 */

#include "ShamirMC.h"

template<class T>
void ShamirMC<T>::POpen_Begin(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    (void) values;
    prepare(S, P);
    P.send_all(os[P.my_num()], true);
}

template<class T>
void ShamirMC<T>::prepare(const vector<T>& S, const Player& P)
{
    os.clear();
    os.resize(P.num_players());
    send = P.my_num() <= threshold;
    if (send)
    {
        for (auto& share : S)
            share.pack(os[P.my_num()]);
    }
}

template<class T>
void ShamirMC<T>::POpen(vector<typename T::clear>& values, const vector<T>& S,
        const Player& P)
{
    prepare(S, P);
    exchange(P);
    finalize(values, S);
}

template<class T>
void ShamirMC<T>::exchange(const Player& P)
{
    for (int offset = 1; offset < P.num_players(); offset++)
    {
        int send_to = P.get_player(offset);
        int receive_from = P.get_player(-offset);
        bool receive = receive_from <= threshold;
        if (send)
            if (receive)
                P.pass_around(os[P.my_num()], os[receive_from], offset);
            else
                P.send_to(send_to, os[P.my_num()], true);
        else if (receive)
            P.receive_player(receive_from, os[receive_from], true);
    }
}

template<class T>
void ShamirMC<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    P.receive_all(os);
    finalize(values, S);
}

template<class T>
void ShamirMC<T>::finalize(vector<typename T::clear>& values,
        const vector<T>& S)
{
    int n_relevant_players = ShamirMachine::s().threshold + 1;
    if (reconstruction.empty())
    {
        reconstruction.resize(n_relevant_players, 1);
        for (int i = 0; i < n_relevant_players; i++)
            reconstruction[i] = Shamir<typename T::clear::Scalar>::get_rec_factor(i,
                    n_relevant_players);
    }

    values.clear();
    values.resize(S.size());
    for (size_t i = 0; i < values.size(); i++)
        for (int j = 0; j < n_relevant_players; j++)
            values[i] += os[j].template get<typename T::clear>() * reconstruction[j];
}
