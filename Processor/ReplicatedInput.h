/*
 * ReplicatedInput.h
 *
 */

#ifndef PROCESSOR_REPLICATEDINPUT_H_
#define PROCESSOR_REPLICATEDINPUT_H_

#include "Auth/ReplicatedMC.h"
#include "Input.h"

class Processor;

template <class T>
class ReplicatedInput : public InputBase<typename T::value_type>
{
    Processor& proc;
    vector<T> shares;

public:
    ReplicatedInput(Processor& proc, ReplicatedMC<T>& MC) : InputBase<typename T::value_type>(proc), proc(proc) { (void)MC; }

    void start(int player, int n_inputs);
    void stop(int player, vector<int> targets);
};

#endif /* PROCESSOR_REPLICATEDINPUT_H_ */
