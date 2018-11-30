/*
 * MaliciousRepMC.h
 *
 */

#ifndef AUTH_MALICIOUSREPMC_H_
#define AUTH_MALICIOUSREPMC_H_

#include "ReplicatedMC.h"
#include "GC/MaliciousRepSecret.h"
#include "GC/Machine.h"

template<class T>
class MaliciousRepMC : public ReplicatedMC<T>
{
protected:
    typedef ReplicatedMC<T> super;

public:
    virtual void POpen_Begin(vector<typename T::clear>& values,
            const vector<T>& S, const Player& P);
    virtual void POpen_End(vector<typename T::clear>& values,
            const vector<T>& S, const Player& P);

    virtual void Check(const Player& P);
};

template<class T>
class HashMaliciousRepMC : public MaliciousRepMC<T>
{
    crypto_generichash_state* hash_state;

    octetStream os;

public:
    // emulate MAC_Check
    HashMaliciousRepMC(const typename T::value_type& _, int __ = 0, int ___ = 0) : HashMaliciousRepMC()
    { (void)_; (void)__; (void)___; }

    // emulate Direct_MAC_Check
    HashMaliciousRepMC(const typename T::value_type& _, Names& ____, int __ = 0, int ___ = 0) : HashMaliciousRepMC()
    { (void)_; (void)__; (void)___; (void)____; }

    HashMaliciousRepMC();
    ~HashMaliciousRepMC();

    void POpen_End(vector<typename T::clear>& values,const vector<T>& S,const Player& P);

    void Check(const Player& P);
};

template<class T>
class CommMaliciousRepMC : public MaliciousRepMC<T>
{
    vector<octetStream> os;

public:
    void POpen_Begin(vector<typename T::clear>& values, const vector<T>& S,
            const Player& P);
    void POpen_End(vector<typename T::clear>& values, const vector<T>& S,
            const Player& P);

    void Check(const Player& P);
};

#endif /* AUTH_MALICIOUSREPMC_H_ */
