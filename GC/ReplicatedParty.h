/*
 * ReplicatedParty.h
 *
 */

#ifndef GC_REPLICATEDPARTY_H_
#define GC_REPLICATEDPARTY_H_

#include "Auth/ReplicatedMC.h"
#include "ReplicatedSecret.h"
#include "Processor.h"
#include "Program.h"
#include "Memory.h"
#include "ThreadMaster.h"

namespace GC
{

class ReplicatedParty : public ThreadMaster<ReplicatedSecret>
{
public:
    static Thread<ReplicatedSecret>& s();

    ReplicatedParty(int argc, const char** argv);
};

inline Thread<ReplicatedSecret>& ReplicatedParty::s()
{
    return Thread<ReplicatedSecret>::s();
}

}

#endif /* GC_REPLICATEDPARTY_H_ */
