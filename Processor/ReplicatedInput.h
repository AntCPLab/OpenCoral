/*
 * ReplicatedInput.h
 *
 */

#ifndef PROCESSOR_REPLICATEDINPUT_H_
#define PROCESSOR_REPLICATEDINPUT_H_

#include "Input.h"

template <class T>
class ReplicatedInput : public InputBase<T>
{
    SubProcessor<T>& proc;
    vector<T> shares;
    vector<octetStream> os;

public:
    ReplicatedInput(SubProcessor<T>& proc, ReplicatedMC<T>& MC) :
            InputBase<T>(proc.Proc), proc(proc)
    {
        (void) MC;
    }

    void reset(int player);
    void add_mine(const typename T::clear& input);
    void add_other(int player);
    void send_mine();

    void start(int player, int n_inputs);
    void stop(int player, vector<int> targets);
};

#endif /* PROCESSOR_REPLICATEDINPUT_H_ */
