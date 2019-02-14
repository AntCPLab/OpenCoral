/*
 * ShamirInput.cpp
 *
 */

#include "ShamirInput.h"
#include "Machines/ShamirMachine.h"

template<class U>
void ShamirInput<U>::reset(int player)
{
    if (player == P.my_num())
    {
        this->shares.clear();
        this->i_share = 0;
        os.clear();
        os.resize(P.num_players());
    }
}

template<class T>
void ShamirInput<T>::add_mine(const typename T::clear& input)
{
    int n = P.num_players();
    int t = ShamirMachine::s().threshold;
    if (vandermonde.empty())
    {
        vandermonde.resize(n, vector<typename T::clear>(t));
        for (int i = 0; i < n; i++)
        {
            typename T::clear x = 1;
            for (int j = 0; j < t; j++)
            {
                x *= (i + 1);
                vandermonde[i][j] = x;
            }
        }
    }

    randomness.resize(t);
    for (auto& x : randomness)
        x.randomize(secure_prng);

    for (int i = 0; i < n; i++)
    {
        typename T::clear x = input;
        for (int j = 0; j < t; j++)
            x += randomness[j] * vandermonde[i][j];
        if (i == P.my_num())
            this->shares.push_back(x);
        else
            x.pack(os[i]);
    }
}

template<class U>
void ShamirInput<U>::add_other(int player)
{
    (void) player;
}

template<class U>
void ShamirInput<U>::send_mine()
{
    for (int i = 0; i < P.num_players(); i++)
        if (i != P.my_num())
            P.send_to(i, os[i], true);
}

template<class T>
void ShamirInput<T>::finalize_other(int player, T& target, octetStream& o)
{
    (void) player;
    target.unpack(o);
}
