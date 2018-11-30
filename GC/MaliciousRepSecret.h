/*
 * MaliciousRepSecret.h
 *
 */

#ifndef GC_MALICIOUSREPSECRET_H_
#define GC_MALICIOUSREPSECRET_H_

#include "ReplicatedSecret.h"

template<class T> class MaliciousRepMC;

namespace GC
{

class MaliciousRepThread;

class MaliciousRepSecret : public ReplicatedSecret<MaliciousRepSecret>
{
    typedef ReplicatedSecret<MaliciousRepSecret> super;

public:
    typedef MaliciousRepSecret DynamicType;

    typedef MaliciousRepMC<MaliciousRepSecret> MC;

    MaliciousRepSecret() {}
    template<class T>
    MaliciousRepSecret(const T& other) : super(other) {}
};

}

#endif /* GC_MALICIOUSREPSECRET_H_ */
