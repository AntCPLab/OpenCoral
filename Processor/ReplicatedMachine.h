/*
 * ReplicatedMachine.h
 *
 */

#ifndef PROCESSOR_REPLICATEDMACHINE_H_
#define PROCESSOR_REPLICATEDMACHINE_H_

#include <string>
using namespace std;

template<class T, class U>
class ReplicatedMachine
{
public:
    ReplicatedMachine(int argc, const char** argv, string name,
            ez::ezOptionParser& opt, int nplayers = 3);
};

#endif /* PROCESSOR_REPLICATEDMACHINE_H_ */
