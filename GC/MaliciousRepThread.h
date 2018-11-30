/*
 * MalicousRepParty.h
 *
 */

#ifndef GC_MALICIOUSREPTHREAD_H_
#define GC_MALICIOUSREPTHREAD_H_

#include "Thread.h"
#include "MaliciousRepSecret.h"
#include "Processor/Data_Files.h"

#include <array>

namespace GC
{

class MaliciousRepThread : public Thread<MaliciousRepSecret>
{
    static thread_local MaliciousRepThread* singleton;

    vector<MaliciousRepSecret> shares;
    vector<BitVec> opened;
    vector<array<MaliciousRepSecret, 3>> triples;

public:
    static MaliciousRepThread& s();

    DataPositions usage;
    Sub_Data_Files<MaliciousRepSecret> DataF;

    MaliciousRepThread(int i, ThreadMaster<MaliciousRepSecret>& master);
    virtual ~MaliciousRepThread() {}

    MaliciousRepSecret::MC* new_mc();

    void pre_run();
    void post_run();

    void and_(Processor<MaliciousRepSecret>& processor, const vector<int>& args, bool repeat);
};

inline MaliciousRepThread& MaliciousRepThread::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton");
}

} /* namespace GC */

#endif /* GC_MALICIOUSREPTHREAD_H_ */
