/*
 * MaliciousShamirMC.h
 *
 */

#ifndef PROTOCOLS_MALICIOUSSHAMIRMC_H_
#define PROTOCOLS_MALICIOUSSHAMIRMC_H_

#include "ShamirMC.h"

template<class T>
class MaliciousShamirMC : public ShamirMC<T>
{
    vector<vector<typename T::open_type::Scalar>> reconstructions;

    vector<typename T::open_type> shares;

    void finalize(vector<typename T::open_type>& values, const vector<T>& S,
            const Player& P);

public:
    MaliciousShamirMC();

    // emulate MAC_Check
    MaliciousShamirMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) :
            MaliciousShamirMC()
    { (void)_; (void)__; (void)___; }

    // emulate Direct_MAC_Check
    MaliciousShamirMC(const typename T::mac_key_type& _, Names& ____, int __ = 0,
            int ___ = 0) :
            MaliciousShamirMC()
    { (void)_; (void)__; (void)___; (void)____; }

    void init_open(const Player& P, int n = 0);
    typename T::open_type finalize_open();
};

#endif /* PROTOCOLS_MALICIOUSSHAMIRMC_H_ */
