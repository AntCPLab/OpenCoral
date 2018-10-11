/*
 * ReplicatedPrivateOutput.h
 *
 */

#ifndef PROCESSOR_REPLICATEDPRIVATEOUTPUT_H_
#define PROCESSOR_REPLICATEDPRIVATEOUTPUT_H_

class Processor;

template <class T>
class ReplicatedPrivateOutput
{
    Processor& proc;

public:
    ReplicatedPrivateOutput(Processor& proc) : proc(proc) {}

    void start(int player, int target, int source);
    void stop(int player, int source);
};

#endif /* PROCESSOR_REPLICATEDPRIVATEOUTPUT_H_ */
