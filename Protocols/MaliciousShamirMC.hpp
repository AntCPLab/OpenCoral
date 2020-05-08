/*
 * MaliciousShamirMC.cpp
 *
 */

#include "MaliciousShamirMC.h"
#include "Machines/ShamirMachine.h"

template<class T>
MaliciousShamirMC<T>::MaliciousShamirMC()
{
    this->threshold = 2 * ShamirMachine::s().threshold;
}

template<class T>
void MaliciousShamirMC<T>::init_open(const Player& P, int n)
{
    int threshold = ShamirMachine::s().threshold;
    if (reconstructions.empty())
    {
        reconstructions.resize(2 * threshold + 2);
        for (int i = threshold + 1; i <= 2 * threshold + 1; i++)
        {
            reconstructions[i].resize(i);
            for (int j = 0; j < i; j++)
                reconstructions[i][j] =
                        Shamir<T>::get_rec_factor(j, i);
        }
    }

    ShamirMC<T>::init_open(P, n);
}

template<class T>
typename T::open_type MaliciousShamirMC<T>::finalize_open()
{
    int threshold = ShamirMachine::s().threshold;
    shares.resize(2 * threshold + 1);
    for (size_t j = 0; j < shares.size(); j++)
        shares[j].unpack((*this->os)[j]);
    typename T::open_type value = 0;
    for (int j = 0; j < threshold + 1; j++)
        value += shares[j] * reconstructions[threshold + 1][j];
    for (int j = threshold + 2; j <= 2 * threshold + 1; j++)
    {
        typename T::open_type check = 0;
        for (int k = 0; k < j; k++)
            check += shares[k] * reconstructions[j][k];
        if (check != value)
            throw mac_fail();
    }
    return value;
}
