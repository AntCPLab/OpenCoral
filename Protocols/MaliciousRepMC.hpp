/*
 * MaliciousRepMC.cpp
 *
 */

#ifndef PROTOCOLS_MALICIOUSREPMC_HPP_
#define PROTOCOLS_MALICIOUSREPMC_HPP_

#include "MaliciousRepMC.h"
#include "GC/Machine.h"
#include "Math/BitVec.h"

#include "ReplicatedMC.hpp"
#include "MAC_Check_Base.hpp"

#include <stdlib.h>

template<class T>
void MaliciousRepMC<T>::POpen_Begin(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    super::POpen_Begin(values, S, P);
}

template<class T>
void MaliciousRepMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    (void)values, (void)S, (void)P;
    throw runtime_error("use subclass");
}

template<class T>
void MaliciousRepMC<T>::POpen(vector<typename T::open_type>&,
        const vector<T>&, const Player&)
{
    throw runtime_error("use subclass");
}

template<class T>
void MaliciousRepMC<T>::Check(const Player& P)
{
    (void)P;
    throw runtime_error("use subclass");
}

template<class T>
HashMaliciousRepMC<T>::HashMaliciousRepMC()
{
    // deal with alignment issues
    int error = posix_memalign((void**)&hash_state, 64, sizeof(crypto_generichash_state));
    if (error)
        throw runtime_error(string("failed to allocate hash state: ") + strerror(error));
    reset();
}

template<class T>
void HashMaliciousRepMC<T>::reset()
{
    crypto_generichash_init(hash_state, 0, 0, crypto_generichash_BYTES);
    needs_checking = false;
}

template<class T>
HashMaliciousRepMC<T>::~HashMaliciousRepMC()
{
    free(hash_state);
}

template<class T>
void HashMaliciousRepMC<T>::POpen(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    ReplicatedMC<T>::POpen(values, S, P);
    finalize(values);
}

template<class T>
void HashMaliciousRepMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    ReplicatedMC<T>::POpen_End(values, S, P);
    finalize(values);
}

template<class T>
void HashMaliciousRepMC<T>::finalize(const vector<typename T::open_type>& values)
{
    os.reset_write_head();
    for (auto& value : values)
        value.pack(os);
    update();
}

template<class T>
void HashMaliciousRepMC<T>::update()
{
    crypto_generichash_update(hash_state, os.get_data(), os.get_length());
    needs_checking = true;
}

template<class T>
void HashMaliciousRepMC<T>::CheckFor(const typename T::open_type& value,
        const vector<T>& check_shares, const Player& P)
{
    os.reset_write_head();
    for (auto& share : check_shares)
    {
        typename T::value_type shares[3];
        for (int i = 0; i < 3; i++)
        {
            int j = (i + P.my_num()) % 3;
            if (j < 2)
                shares[i] = share[j];
            else
                shares[i] = value - share.sum();
        }
        for (auto& x : shares)
            x.pack(os);
    }
    update();
    Check(P);
}

template<class T>
void HashMaliciousRepMC<T>::Check(const Player& P)
{
    if (needs_checking)
    {
        unsigned char hash[crypto_generichash_BYTES];
        crypto_generichash_final(hash_state, hash, sizeof hash);
        reset();
        vector<octetStream> os(P.num_players());
        os[P.my_num()].serialize(hash);
        P.Broadcast_Receive(os);
        for (int i = 0; i < P.num_players(); i++)
            if (os[i] != os[P.my_num()])
                throw mac_fail();
    }
}

template<class T>
void CommMaliciousRepMC<T>::POpen(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    POpen_Begin(values, S, P);
    POpen_End(values, S, P);
}

template<class T>
void CommMaliciousRepMC<T>::POpen_Begin(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    assert(T::length == 2);
    (void)values;
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    for (auto& x : S)
        for (int i = 0; i < 2; i++)
            x[i].pack(os[1 - i]);
    P.pass_around(os[0], 1);
    P.pass_around(os[1], 2);
}

template<class T>
void CommMaliciousRepMC<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    (void) P;
    if (os[0] != os[1])
        throw mac_fail();
    values.clear();
    for (auto& x : S)
        values.push_back(os[0].template get<BitVec>() + x.sum());
}

template<class T>
void CommMaliciousRepMC<T>::Check(const Player& P)
{
    (void)P;
}

#endif
