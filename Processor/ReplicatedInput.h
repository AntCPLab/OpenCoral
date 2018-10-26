/*
 * ReplicatedInput.h
 *
 */

#ifndef PROCESSOR_REPLICATEDINPUT_H_
#define PROCESSOR_REPLICATEDINPUT_H_

#include "Auth/ReplicatedMC.h"
#include "Input.h"

template <class T>
class ReplicatedInput : public InputBase<typename T::value_type>
{
    SubProcessor<T>& proc;
    vector<T> shares;

public:
    ReplicatedInput(SubProcessor<T>& proc, ReplicatedMC<T>& MC) :
            InputBase<typename T::value_type>(proc.Proc), proc(proc)
    {
        (void) MC;
    }

    void start(int player, int n_inputs);
    void stop(int player, vector<int> targets);
};

#endif /* PROCESSOR_REPLICATEDINPUT_H_ */
