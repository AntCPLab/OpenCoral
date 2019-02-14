/*
 * ShamirMC.h
 *
 */

#ifndef AUTH_SHAMIRMC_H_
#define AUTH_SHAMIRMC_H_

#include "MAC_Check.h"
#include "Math/ShamirShare.h"
#include "Machines/ShamirMachine.h"

template<class T>
class ShamirMC : public MAC_Check_Base<T>
{
    vector<typename T::clear> reconstruction;

protected:
    vector<octetStream> os;
    int threshold;

public:
    ShamirMC() : threshold(ShamirMachine::s().threshold) {}

    // emulate MAC_Check
    ShamirMC(const typename T::value_type& _, int __ = 0, int ___ = 0) : ShamirMC()
    { (void)_; (void)__; (void)___; }

    // emulate Direct_MAC_Check
    ShamirMC(const typename T::value_type& _, Names& ____, int __ = 0, int ___ = 0) :
        ShamirMC()
    { (void)_; (void)__; (void)___; (void)____; }

    void POpen_Begin(vector<typename T::clear>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::clear>& values,const vector<T>& S,const Player& P);

    void Check(const Player& P) { (void)P; }
};

#endif /* AUTH_SHAMIRMC_H_ */
