/*
 * MaliciousRepSecret.h
 *
 */

#ifndef GC_MALICIOUSREPSECRET_H_
#define GC_MALICIOUSREPSECRET_H_

#include "ShareSecret.h"
#include "Machine.h"
#include "Protocols/Beaver.h"
#include "Protocols/MaliciousRepMC.h"
#include "Processor/DummyProtocol.h"

template<class T> class MaliciousRepMC;

namespace GC
{

template<class T> class ShareThread;
template<class T> class RepPrep;

class MaliciousRepSecret : public ReplicatedSecret<MaliciousRepSecret>
{
    typedef ReplicatedSecret<MaliciousRepSecret> super;

public:
    typedef Memory<MaliciousRepSecret> DynamicMemory;

    typedef MaliciousRepMC<MaliciousRepSecret> MC;
    typedef MC MAC_Check;

    typedef Beaver<MaliciousRepSecret> Protocol;
    typedef ReplicatedInput<MaliciousRepSecret> Input;
    typedef RepPrep<MaliciousRepSecret> LivePrep;

    static MC* new_mc(Machine<MaliciousRepSecret>& machine)
    {
        if (machine.more_comm_less_comp)
            return new CommMaliciousRepMC<MaliciousRepSecret>;
        else
            return new HashMaliciousRepMC<MaliciousRepSecret>;
    }

    static MaliciousRepSecret constant(const BitVec& other, int my_num, const BitVec& alphai)
    {
        (void) my_num, (void) alphai;
        return other;
    }

    MaliciousRepSecret() {}
    template<class T>
    MaliciousRepSecret(const T& other) : super(other) {}
};

}

#endif /* GC_MALICIOUSREPSECRET_H_ */
