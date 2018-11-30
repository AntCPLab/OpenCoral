/*
 * ReplicatedParty.h
 *
 */

#ifndef GC_REPLICATEDPARTY_H_
#define GC_REPLICATEDPARTY_H_

#include "Auth/ReplicatedMC.h"
#include "Auth/MaliciousRepMC.h"
#include "ReplicatedSecret.h"
#include "Processor.h"
#include "Program.h"
#include "Memory.h"
#include "ThreadMaster.h"

namespace GC
{

template<class T>
class ReplicatedParty : public ThreadMaster<T>
{
    ez::ezOptionParser opt;
    OnlineOptions online_opts;

public:
    static Thread<T>& s();

    ReplicatedParty(int argc, const char** argv);

    Thread<T>* new_thread(int i);

    void post_run();
};

template<class T>
inline Thread<T>& ReplicatedParty<T>::s()
{
    return Thread<T>::s();
}

}

#endif /* GC_REPLICATEDPARTY_H_ */
