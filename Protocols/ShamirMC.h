/*
 * ShamirMC.h
 *
 */

#ifndef PROTOCOLS_SHAMIRMC_H_
#define PROTOCOLS_SHAMIRMC_H_

#include "MAC_Check_Base.h"
#include "Protocols/ShamirShare.h"
#include "Machines/ShamirMachine.h"

template<class T>
class ShamirMC : public MAC_Check_Base<T>
{
    vector<typename T::clear::Scalar> reconstruction;

    bool send;

    void finalize(vector<typename T::clear>& values, const vector<T>& S);

protected:
    vector<octetStream> os;
    int threshold;

    void prepare(const vector<T>& S, const Player& P);
    void exchange(const Player& P);

public:
    ShamirMC() : threshold(ShamirMachine::s().threshold) {}

    // emulate MAC_Check
    ShamirMC(const typename T::mac_key_type& _, int __ = 0, int ___ = 0) : ShamirMC()
    { (void)_; (void)__; (void)___; }

    // emulate Direct_MAC_Check
    ShamirMC(const typename T::mac_key_type& _, Names& ____, int __ = 0, int ___ = 0) :
        ShamirMC()
    { (void)_; (void)__; (void)___; (void)____; }

    void POpen(vector<typename T::clear>& values,const vector<T>& S,const Player& P);
    void POpen_Begin(vector<typename T::clear>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::clear>& values,const vector<T>& S,const Player& P);

    void Check(const Player& P) { (void)P; }
};

#endif /* PROTOCOLS_SHAMIRMC_H_ */
