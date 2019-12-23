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
class ShareThread
{
    static thread_local ShareThread<T>* singleton;

public:
    static ShareThread& s();

    Player* P;
    typename T::MC* MC;
    typename T::Protocol* protocol;

    DataPositions usage;
    Preprocessing<T>& DataF;

    ShareThread(const Names& N, OnlineOptions& opts);
    virtual ~ShareThread();

    virtual typename T::MC* new_mc(typename T::mac_key_type mac_key)
    { return T::new_mc(mac_key); }

    void pre_run(Player& P, typename T::mac_key_type mac_key);
    void post_run();

    void and_(Processor<T>& processor, const vector<int>& args, bool repeat);
};

template<class T>
class StandaloneShareThread : public ShareThread<T>, public Thread<T>
{
public:
    StandaloneShareThread(int i, ThreadMaster<T>& master);

    void pre_run();
    void post_run() { ShareThread<T>::post_run(); }
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
