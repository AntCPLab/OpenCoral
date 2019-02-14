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
void MaliciousShamirMC<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    (void) P;
    int threshold = ShamirMachine::s().threshold;
    if (reconstructions.empty())
    {
        reconstructions.resize(2 * threshold + 2);
        for (int i = threshold + 1; i <= 2 * threshold + 1; i++)
        {
            reconstructions[i].resize(i);
            for (int j = 0; j < i; j++)
                reconstructions[i][j] = Shamir<T>::get_rec_factor(j, i);
        }
    }

    values.clear();
    values.resize(S.size());
    vector<T> shares(2 * threshold + 1);
    for (size_t i = 0; i < values.size(); i++)
    {
        for (size_t j = 0; j < shares.size(); j++)
            shares[j].unpack(this->os[j]);
        T value = 0;
        for (int j = 0; j < threshold + 1; j++)
            value += shares[j] * reconstructions[threshold + 1][j];
        for (int j = threshold + 2; j <= 2 * threshold + 1; j++)
        {
            T check = 0;
            for (int k = 0; k < j; k++)
                check += shares[k] * reconstructions[j][k];
            if (check != value)
                throw mac_fail();
        }
        values[i] = value;
    }
}
