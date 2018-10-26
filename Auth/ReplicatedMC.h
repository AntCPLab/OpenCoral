/*
 * ReplicatedMC.h
 *
 */

#ifndef AUTH_REPLICATEDMC_H_
#define AUTH_REPLICATEDMC_H_

#include "MAC_Check.h"

template <class T>
class ReplicatedMC : public MAC_Check_Base<typename T::value_type>
{
public:
    // emulate MAC_Check
    ReplicatedMC(const gfp& _ = {}, int __ = 0, int ___ = 0) :
        MAC_Check_Base<typename T::value_type>({})
    { (void)_; (void)__; (void)___; }

    // emulate Direct_MAC_Check
    ReplicatedMC(const gfp& _, Names& ____, int __ = 0, int ___ = 0) :
        MAC_Check_Base<typename T::value_type>({})
    { (void)_; (void)__; (void)___; (void)____; }

    void POpen_Begin(vector<typename T::clear>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::clear>& values,const vector<T>& S,const Player& P);

    void Check(const Player& P) { (void)P; }
};

#endif /* AUTH_REPLICATEDMC_H_ */
