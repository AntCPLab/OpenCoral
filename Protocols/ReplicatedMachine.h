/*
 * ReplicatedMachine.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDMACHINE_H_
#define PROTOCOLS_REPLICATEDMACHINE_H_

#include <string>
using namespace std;

template<class T, class U>
class ReplicatedMachine
{
public:
    ReplicatedMachine(int argc, const char** argv, string name,
            ez::ezOptionParser& opt, int nplayers = 3);
    ReplicatedMachine(int argc, const char** argv, ez::ezOptionParser& opt,
            int nplayers = 3) :
            ReplicatedMachine(argc, argv, "", opt, nplayers)
    {
    }
};

#endif /* PROTOCOLS_REPLICATEDMACHINE_H_ */
