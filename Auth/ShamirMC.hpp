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
    os.clear();
    os.resize(P.num_players());
    if (P.my_num() <= threshold)
    {
        for (auto& share : S)
            share.pack(os[P.my_num()]);
        for (int i = 0; i < P.num_players(); i++)
            if (i != P.my_num())
                P.send_to(i, os[P.my_num()], true);
    }
    for (int i = 0; i <= threshold; i++)
        if (i != P.my_num())
            P.receive_player(i, os[i], true);
}

template<class T>
void ShamirMC<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    (void) P;
    int n_relevant_players = ShamirMachine::s().threshold + 1;
    if (reconstruction.empty())
    {
        reconstruction.resize(n_relevant_players, 1);
        for (int i = 0; i < n_relevant_players; i++)
            reconstruction[i] = Shamir<typename T::clear>::get_rec_factor(i,
                    n_relevant_players);
    }

    values.clear();
    values.resize(S.size());
    for (size_t i = 0; i < values.size(); i++)
        for (int j = 0; j < n_relevant_players; j++)
            values[i] += os[j].template get<typename T::clear>() * reconstruction[j];
}
