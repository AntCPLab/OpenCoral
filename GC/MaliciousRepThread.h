/*
 * MalicousRepParty.h
 *
 */

#ifndef GC_MALICIOUSREPTHREAD_H_
#define GC_MALICIOUSREPTHREAD_H_

#include "Thread.h"
#include "MaliciousRepSecret.h"
#include "MaliciousRepPrep.h"
#include "Processor/Data_Files.h"

#include <array>

namespace GC
{

class MaliciousRepThread : public Thread<MaliciousRepSecret>
{
    static thread_local MaliciousRepThread* singleton;

public:
    static MaliciousRepThread& s();

    DataPositions usage;
    MaliciousRepPrep DataF;

    MaliciousRepThread(int i, ThreadMaster<MaliciousRepSecret>& master);
    virtual ~MaliciousRepThread() {}

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
