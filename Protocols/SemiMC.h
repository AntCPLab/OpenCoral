/*
 * SemiMC.h
 *
 */

#ifndef PROTOCOLS_SEMIMC_H_
#define PROTOCOLS_SEMIMC_H_

#include "MAC_Check.h"
#include "Tools/Bundle.h"

template<class T>
class SemiMC : public TreeSum<typename T::open_type>, public MAC_Check_Base<T>
{
public:
    // emulate MAC_Check
    SemiMC(const typename T::mac_key_type& _ = {}, int __ = 0, int ___ = 0)
    { (void)_; (void)__; (void)___; }

    void POpen_Begin(vector<typename T::open_type>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P);

    void Check(const Player& P) { (void)P; }
};

template<class T>
class DirectSemiMC : public SemiMC<T>
{
public:
    // emulate Direct_MAC_Check
    DirectSemiMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0)
    { (void)_; (void)__; (void)___; (void)____; }

    void POpen_Begin(vector<typename T::open_type>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P);

    void Check(const Player& P) { (void)P; }
};

#endif /* PROTOCOLS_SEMIMC_H_ */
