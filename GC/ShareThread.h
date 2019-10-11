/*
 * MalicousRepParty.h
 *
 */

#ifndef GC_SHARETHREAD_H_
#define GC_SHARETHREAD_H_

#include "Thread.h"
#include "MaliciousRepSecret.h"
#include "RepPrep.h"
#include "SemiHonestRepPrep.h"
#include "Processor/Data_Files.h"
#include "Protocols/ReplicatedInput.h"

#include <array>

namespace GC
{

template<class T>
class ShareThread : public Thread<T>
{
    static thread_local ShareThread<T>* singleton;

public:
    static ShareThread& s();

    DataPositions usage;
    Preprocessing<T>& DataF;

    ShareThread(int i, ThreadMaster<T>& master);
    virtual ~ShareThread();

    void pre_run();
    void post_run();

    void and_(Processor<T>& processor, const vector<int>& args, bool repeat);
};

template<class T>
thread_local ShareThread<T>* ShareThread<T>::singleton = 0;

template<class T>
inline ShareThread<T>& ShareThread<T>::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton");
}

} /* namespace GC */

#endif /* GC_SHARETHREAD_H_ */
